using NaughtyAttributes;
using NetworkProtocol;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using TMPro;


public class GameManager : MonoBehaviour
{
    [SerializeField, BoxGroup("Init")]
    private GameRefSCO SCORef;
    [SerializeField, BoxGroup("Init")]
    private PlayerInputManager inputManager;

    [SerializeField, BoxGroup("Init")]
    private GameObject clientCarPrefab;
    [SerializeField, BoxGroup("Init")]
    private CarController ownCarController;

    [SerializeField, BoxGroup("Init")]
    private List<Transform> infectedSpawn = new List<Transform>();
    [SerializeField, BoxGroup("Init")]
    private List<Transform> survivorSpawn = new List<Transform>();

    [SerializeField, BoxGroup("UI")]
    private TextMeshProUGUI speedTxt;
    [SerializeField, BoxGroup("UI")]
    private TextMeshProUGUI infoTxt;

    private Player m_ownPlayer = null;
    private float m_gameDuration;

    private bool m_gameEnded = false;

    //private List<PlayerInput> m_playerInputs = new List<PlayerInput>();
    //private UInt16 m_inputIndex = 1;

    private void Awake()
    {
        Physics.simulationMode = SimulationMode.Script;

        SCORef.Game = this;
        SCORef.GameData.state = GameData.GameState.WAITING_GAME_START;
        return;

        // Manually simutale physics
        Physics.simulationMode = SimulationMode.Script;

        SCORef.Game = this;
        SCORef.GameData.state = GameData.GameState.WAITING_GAME_START;

        m_ownPlayer = SCORef.GameData.players.Find((Player player) => { return player.Index == SCORef.GameData.ownPlayerIndex; });

        foreach (Player player in SCORef.GameData.players)
        {
            if (player.Index == SCORef.GameData.ownPlayerIndex)
            {
                player.carController = ownCarController;
                if (player.isInfected)
                {
                    ownCarController.transform.position = infectedSpawn[player.slotId].position;
                    ownCarController.transform.rotation = infectedSpawn[player.slotId].rotation;
                    ownCarController.InitController(true);
                }
                else
                {
                    ownCarController.transform.position = survivorSpawn[player.slotId].position;
                    ownCarController.transform.rotation = survivorSpawn[player.slotId].rotation;
                    ownCarController.InitController(false);
                }
            }
            else
            {
                if(player.isInfected)
                {
                    GameObject car = Instantiate<GameObject>(clientCarPrefab, infectedSpawn[player.slotId].position, infectedSpawn[player.slotId].rotation);
                    player.carController = car.GetComponent<CarController>();
                    player.carController.InitController(true, player.Name);
                }
                else
                {
                    GameObject car = Instantiate<GameObject>(clientCarPrefab, survivorSpawn[player.slotId].position, survivorSpawn[player.slotId].rotation);
                    player.carController = car.GetComponent<CarController>();
                    player.carController.InitController(false, player.Name);
                }
            }
        }
    }

    private void FixedUpdate()
    {
        if (m_gameEnded)
            return;

        if (inputManager != null && SCORef != null && SCORef.Network != null)
        {
            PlayerInputPacket packet = new PlayerInputPacket();
            packet.inputs = inputManager.m_lastInput;

            // packet.inputIndex = m_inputIndex++;
            // m_playerInputs.Add(inputManager.m_lastInput);

            SCORef.Network.BuildAndSendPacketToNetwork<PlayerInputPacket>(packet, ENet6.PacketFlags.None, 0);
        }

        /*foreach (Player player in SCORef.GameData.players)
        {
            if (player.carController != null)
                player.carController.UpdatePhysics();
        }

        Physics.Simulate(Time.fixedDeltaTime);
        speedTxt.text = ownCarController.ShowSpeed.ToString();*/
    }

    public void HandleMessage(byte[] message)
    {
        List<byte> byteArray = new List<byte>(message);

        int offset = 0;
        EOpcode opcode = (EOpcode)Serializer.Deserialize_uByte(byteArray, ref offset);
        switch (opcode)
        {
            case EOpcode.S_PlayerDisconnected:
                {
                    PlayerDisconnectedPacket disconnectPacket = PlayerDisconnectedPacket.Deserialize(byteArray, ref offset);
                    Player targetPlayer = SCORef.GameData.players.Find((Player player) => { return player.Index == disconnectPacket.playerIndex; });
                    if (targetPlayer != null)
                    {
                        Destroy(targetPlayer.carController.gameObject);
                        SCORef.GameData.players.Remove(targetPlayer);
                    }
                    else
                        Debug.LogWarning("Can't find disconnected player");
                }
                break;

            case EOpcode.S_StartMovingState:
                {
                    GameStateStartMovePacket gameStateStartMovePacket = GameStateStartMovePacket.Deserialize(byteArray, ref offset);

                    if (m_ownPlayer.isInfected)
                    {
                        if (gameStateStartMovePacket.moveInfected)
                        {
                            inputManager.EnableCarMap();
                            StopAllCoroutines();
                            StartCoroutine(GoMsg(false));
                        }
                        else
                            StartCoroutine(InitCountDown());

                    }
                    else if (!gameStateStartMovePacket.moveInfected)
                    {
                        inputManager.EnableCarMap();
                        StartCoroutine(GoMsg(true));
                    }
                }
                break;

            case EOpcode.S_StartGameState:
                {
                    GameStateStartPacket gameStateStartPacket = GameStateStartPacket.Deserialize(byteArray, ref offset);
                    m_gameDuration = gameStateStartPacket.gameDuration / 1000.0f;
                    StartCoroutine(CountDown());
                }
                break;

            case EOpcode.S_FinishedState:
                {
                    GameStateFinishPacket gameStateFinishPacket = GameStateFinishPacket.Deserialize(byteArray, ref offset);
                    if (gameStateFinishPacket.infectedWins)
                        infoTxt.text = "THE INFECTED WINS";
                    else
                        infoTxt.text = "THE SURVIVORS WINS";

                    m_gameEnded = true;
                    inputManager.DisableCarMap();
                    StartCoroutine(ReturnToLobby());
                }
                break;

            case EOpcode.S_PlayersState:
                {
                    PlayersStatePacket playersStatePacket = PlayersStatePacket.Deserialize(byteArray, ref offset);

                    // local
                    ownCarController.gameObject.transform.position = playersStatePacket.localPosition;
                    ownCarController.gameObject.transform.rotation = playersStatePacket.localRotation;

                    if (playersStatePacket.localAtRest)
                    {
                        ownCarController.CarRb.velocity = Vector3.zero;
                        ownCarController.CarRb.angularVelocity = Vector3.zero;

                        ownCarController.FrontLeftWheelVelocity = 0f;
                        ownCarController.FrontRightWheelVelocity = 0f;
                        ownCarController.RearLeftWheelVelocity = 0f;
                        ownCarController.RearRightWheelVelocity = 0f;
                    }
                    else
                    {
                        ownCarController.CarRb.velocity = playersStatePacket.localLinearVelocity;
                        ownCarController.CarRb.angularVelocity = playersStatePacket.localAngularVelocity;

                        ownCarController.FrontLeftWheelVelocity = playersStatePacket.localFrontLeftWheelVelocity;
                        ownCarController.FrontRightWheelVelocity = playersStatePacket.localFrontRightWheelVelocity;
                        ownCarController.RearLeftWheelVelocity = playersStatePacket.localRearLeftWheelVelocity;
                        ownCarController.RearRightWheelVelocity = playersStatePacket.localRearRightWheelVelocity;
                    }

                    foreach (PlayersStatePacket.PlayerState playerState in playersStatePacket.otherPlayerState)
                    {
                        Player targetPlayer = SCORef.GameData.players.Find((Player player) => { return player.Index == playerState.playerIndex; });
                        if (targetPlayer != null && targetPlayer.carController != null)
                        {
                            // position / rotation
                            targetPlayer.carController.gameObject.transform.position = playerState.position;
                            targetPlayer.carController.gameObject.transform.rotation = playerState.rotation;

                            // velocity
                            if (playerState.atRest)
                            {
                                targetPlayer.carController.CarRb.velocity = Vector3.zero;
                                targetPlayer.carController.CarRb.angularVelocity = Vector3.zero;

                                // wheels
                                targetPlayer.carController.FrontLeftWheelVelocity = 0f;
                                targetPlayer.carController.FrontRightWheelVelocity = 0f;
                                targetPlayer.carController.RearLeftWheelVelocity = 0f;
                                targetPlayer.carController.RearRightWheelVelocity = 0f;
                            }
                            else
                            {
                                targetPlayer.carController.CarRb.velocity = playerState.linearVelocity;
                                targetPlayer.carController.CarRb.angularVelocity = playerState.angularVelocity;

                                // wheels
                                targetPlayer.carController.FrontLeftWheelVelocity = playerState.frontLeftWheelVelocity;
                                targetPlayer.carController.FrontRightWheelVelocity = playerState.frontRightWheelVelocity;
                                targetPlayer.carController.RearLeftWheelVelocity = playerState.frontLeftWheelVelocity;
                                targetPlayer.carController.RearRightWheelVelocity = playerState.rearRightWheelVelocity;
                            }
                        }
                    }
                }
                break;

            case EOpcode.S_PlayerInfected:
                {
                    PlayerInfectedPacket playerInfectedPacket = PlayerInfectedPacket.Deserialize(byteArray, ref offset);
                    Player targetPlayer = SCORef.GameData.players.Find((Player player) => { return player.Index == playerInfectedPacket.playerIndex; });

                    if (targetPlayer != null)
                    {
                        targetPlayer.isInfected = true;
                        targetPlayer.carController.SetInfectedModel();
                    }
                    else
                        Debug.LogWarning("Can't find infected player");
                }
                break;

            case EOpcode.S_DebugCollision:
                {
                    DebugCollisionPacket debugCollisionPacket = DebugCollisionPacket.Deserialize(byteArray, ref offset);
                    // TODO
                }
                break;
        }
    }

    public void HandleDisconnection(uint disconectId)
    {
        SCORef.GameData.players.Clear();
        SCORef.GameData.ownPlayerIndex = -1;

        SceneManager.LoadScene(0);
    }

    private IEnumerator InitCountDown()
    {
        infoTxt.text = "3";
        yield return new WaitForSeconds(1);
        infoTxt.text = "2";
        yield return new WaitForSeconds(1);
        infoTxt.text = "1";
    }

    private IEnumerator GoMsg(bool pendingMsg)
    {
        infoTxt.text = "GO";
        yield return new WaitForSeconds(1.5f);
        if(pendingMsg)
            infoTxt.text = "- RESPITE PHASE -";
        else
            infoTxt.gameObject.SetActive(false);
    }

    private IEnumerator CountDown()
    {
        while(m_gameDuration >= 0)
        {
            int seconds = (int)m_gameDuration % 60;
            int minutes = (int)m_gameDuration / 60;

            infoTxt.text = minutes.ToString("D2") + ":" + seconds.ToString("D2");
            --m_gameDuration;
            yield return new WaitForSeconds(1);
        }
    }

    private IEnumerator ReturnToLobby()
    {
        yield return new WaitForSeconds(1.8f);
        SceneManager.LoadScene(0);
    }
}
