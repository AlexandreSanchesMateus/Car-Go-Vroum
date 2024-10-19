using NaughtyAttributes;
using NetworkProtocol;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using TMPro;
using UnityEngine.Playables;
using System.Runtime.CompilerServices;
using UnityEngine.Windows;


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
    [SerializeField, BoxGroup("UI")]
    private TextMeshProUGUI counterTxt;

    [SerializeField, BoxGroup("Network")]
    private int targetJitterBufferSize = 4;

    private Player m_ownPlayer = null;
    private float m_gameDuration;

    private bool m_gameEnded = false;

    private List<PredictedInput> m_predictedState = new List<PredictedInput>();
    private Queue<PlayersStatePacket> m_jitterBuffer = new Queue<PlayersStatePacket>();
    float m_jitterFactor = 1f;
    private UInt16 m_nextInputIndex = 1;

    private void Awake()
    {
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

    private void Update()
    {
        speedTxt.text = ownCarController.ShowSpeed.ToString();

        // jitter buffer
        if (m_jitterBuffer.Count >= 2)
        {
            float interpIncrement = Time.deltaTime / 0.02f;

            if (m_jitterBuffer.Count > targetJitterBufferSize)
                interpIncrement *= 1.0f + (0.05f * (m_jitterBuffer.Count - targetJitterBufferSize));
            else
                interpIncrement *= 1.0f - (0.05f * (targetJitterBufferSize - m_jitterBuffer.Count));

            m_jitterFactor += interpIncrement;

            // interpolate visual


            if (m_jitterFactor >= 1.0f)
            {
                // traite le dernier packet
                PlayersStatePacket packet = m_jitterBuffer.Dequeue();

                bool performReconciliation = false;

                int count = 0;
                PredictedInput predictedInput = null;
                foreach (PredictedInput input in m_predictedState)
                {
                    if (input.inputIndex <= packet.inputIndex)
                        ++count;

                    if(input.inputIndex == packet.inputIndex)
                    {
                        predictedInput = input;
                        break;
                    }
                }

                if (predictedInput != null)
                {
                    // TODO

                }

                if(count > 0)
                    m_predictedState.RemoveRange(0, count);

                if (performReconciliation)
                {
                    // teleport
                    ownCarController.gameObject.transform.position = packet.localPhysicState.position;
                    ownCarController.gameObject.transform.rotation = packet.localPhysicState.rotation;

                    if (packet.localAtRest)
                    {
                        ownCarController.CarRb.velocity = Vector3.zero;
                        ownCarController.CarRb.angularVelocity = Vector3.zero;
                    }
                    else
                    {
                        ownCarController.CarRb.velocity = packet.localPhysicState.linearVelocity;
                        ownCarController.CarRb.angularVelocity = packet.localPhysicState.angularVelocity;

                        ownCarController.FrontLeftWheelVelocity = packet.localPhysicState.frontLeftWheelVelocity;
                        ownCarController.FrontRightWheelVelocity = packet.localPhysicState.frontRightWheelVelocity;
                        ownCarController.RearLeftWheelVelocity = packet.localPhysicState.rearLeftWheelVelocity;
                        ownCarController.RearRightWheelVelocity = packet.localPhysicState.rearRightWheelVelocity;
                    }
                }

                m_jitterFactor -= 1.0f;
            }
        }
    }

    private void FixedUpdate()
    {
        if (m_gameEnded)
            return;

        // update simulation
        foreach (Player player in SCORef.GameData.players)
        {
            if (player.carController != null)
                player.carController.UpdatePhysics();
        }

        Physics.Simulate(Time.fixedDeltaTime);

        if (inputManager != null && SCORef != null && SCORef.Network != null)
        {
            // send input
            PlayerInputPacket packet = new PlayerInputPacket();
            packet.inputs = inputManager.m_lastInput;
            packet.inputIndex = m_nextInputIndex++;

            SCORef.Network.BuildAndSendPacketToNetwork<PlayerInputPacket>(packet, ENet6.PacketFlags.None, 0);

            // store state
            PredictedInput predictedInput = new PredictedInput();
            predictedInput.localInput = inputManager.m_lastInput;
            predictedInput.inputIndex = packet.inputIndex;

            predictedInput.physicState.position = ownCarController.transform.position;
            predictedInput.physicState.rotation = ownCarController.transform.rotation;

            predictedInput.physicState.linearVelocity = ownCarController.CarRb.velocity;
            predictedInput.physicState.angularVelocity = ownCarController.CarRb.angularVelocity;

            predictedInput.physicState.frontLeftWheelVelocity = ownCarController.FrontLeftWheelVelocity;
            predictedInput.physicState.frontRightWheelVelocity = ownCarController.FrontRightWheelVelocity;
            predictedInput.physicState.rearLeftWheelVelocity = ownCarController.RearLeftWheelVelocity;
            predictedInput.physicState.rearRightWheelVelocity = ownCarController.RearRightWheelVelocity;

            foreach (Player player in SCORef.GameData.players)
            {
                PhysicState otherState = new PhysicState();
                otherState.position = player.carController.transform.position;
                otherState.rotation = player.carController.transform.rotation;

                otherState.linearVelocity = player.carController.CarRb.velocity;
                otherState.angularVelocity = player.carController.CarRb.angularVelocity;

                otherState.frontLeftWheelVelocity = player.carController.FrontLeftWheelVelocity;
                otherState.frontRightWheelVelocity = player.carController.FrontRightWheelVelocity;
                otherState.rearLeftWheelVelocity = player.carController.RearLeftWheelVelocity;
                otherState.rearRightWheelVelocity = player.carController.RearRightWheelVelocity;

                predictedInput.others.Add(otherState);
            }
        }
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
                            StartCoroutine(GoMsg());
                        }
                        else
                            StartCoroutine(InitCountDown());
                    }
                    else if (!gameStateStartMovePacket.moveInfected)
                    {
                        inputManager.EnableCarMap();
                        StartCoroutine(GoMsg());
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

                    infoTxt.gameObject.SetActive(true);
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
                    m_jitterBuffer.Enqueue(playersStatePacket);
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

    private IEnumerator GoMsg()
    {
        infoTxt.text = "GO";
        yield return new WaitForSeconds(1.5f);
        infoTxt.gameObject.SetActive(false);
    }

    private IEnumerator CountDown()
    {
        while(m_gameDuration >= 0)
        {
            int seconds = (int)m_gameDuration % 60;
            int minutes = (int)m_gameDuration / 60;

            counterTxt.text = minutes.ToString("D2") + ":" + seconds.ToString("D2");
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
