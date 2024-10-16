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

    private Player ownPlayer = null;

    //private List<PlayerInput> m_playerInputs = new List<PlayerInput>();
    //private UInt16 m_inputIndex = 1;

    private void Awake()
    {
        // Manually simutale physics
        Physics.simulationMode = SimulationMode.Script;
        SCORef.GameData.state = GameData.GameState.WAITING_GAME_START;

        ownPlayer = SCORef.GameData.players.Find((Player player) => { return player.Index == SCORef.GameData.ownPlayerIndex; });

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
        if (inputManager != null && SCORef != null && SCORef.Network != null)
        {
            PlayerInputPacket packet = new PlayerInputPacket();
            packet.inputs = inputManager.m_lastInput;

            // packet.inputIndex = m_inputIndex++;
            // m_playerInputs.Add(inputManager.m_lastInput);

            SCORef.Network.BuildAndSendPacketToNetwork<PlayerInputPacket>(packet, ENet6.PacketFlags.None, 0);
        }

        foreach(Player player in SCORef.GameData.players)
        {
            if(player.carController != null)
                player.carController.UpdatePhysics();
        }

        Physics.Simulate(Time.fixedDeltaTime);
        speedTxt.text = ownCarController.ShowSpeed.ToString();
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
                GameStateStartMovePacket gameStateStartMovePacket = GameStateStartMovePacket.Deserialize(byteArray, ref offset);

                if (ownPlayer.isInfected)
                {
                    if (gameStateStartMovePacket.moveInfected)
                        inputManager.EnableCarMap();
                    else
                        StartCoroutine(CountDown());

                }
                else if(!gameStateStartMovePacket.moveInfected)
                {
                    inputManager.EnableCarMap();
                    StartCoroutine(GoMsg());
                }
                break;

            case EOpcode.S_FinishedState:
                break;

            case EOpcode.S_PlayersState:
                PlayersStatePacket playersStatePacket = PlayersStatePacket.Deserialize(byteArray,ref offset);

                foreach(PlayersStatePacket.PlayerState playerState in playersStatePacket.otherPlayerState)
                {
                    Player targetPlayer = SCORef.GameData.players.Find((Player player) => { return player.Index == playerState.playerIndex; });
                    if(targetPlayer != null)
                    {

                    }
                }

                // traitement des inputs
                // réconciliation
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
                DebugCollisionPacket debugCollisionPacket = DebugCollisionPacket.Deserialize(byteArray, ref offset);

                break;
        }
    }

    public void HandleDisconnection(uint disconectId)
    {
        SCORef.GameData.players.Clear();
        SCORef.GameData.ownPlayerIndex = -1;

        SceneManager.LoadScene(0);
    }

    private IEnumerator CountDown()
    {
        infoTxt.text = "3";
        yield return new WaitForSeconds(1);
        infoTxt.text = "2";
        yield return new WaitForSeconds(1);
        infoTxt.text = "1";
        yield return new WaitForSeconds(1);
        StartCoroutine(GoMsg());
    }

    private IEnumerator GoMsg()
    {
        infoTxt.text = "GO";
        yield return new WaitForSeconds(1.5f);
        infoTxt.gameObject.SetActive(false);
    }
}
