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
    [SerializeField, BoxGroup("UI")]
    private TextMeshProUGUI counterTxt;
    [SerializeField, BoxGroup("UI")]
    private TextMeshProUGUI survivorCountTxt;
    [SerializeField, BoxGroup("UI")]
    private TextMeshProUGUI infectedCountTxt;

    [SerializeField, BoxGroup("Network")]
    private int targetJitterBufferSize = 4;

    private int m_survivorCount, m_infectedCount;

    private Player m_ownPlayer = null;
    private float m_gameDuration;

    private bool m_gameEnded = false;

    private List<PredictedInput> m_predictedState = new List<PredictedInput>();
    private Queue<PlayersStatePacket> m_jitterBuffer = new Queue<PlayersStatePacket>();
    float m_jitterFactor = 1f;
    private UInt16 m_nextInputIndex = 1;

    private void Awake()
    {
        /*int count = 0;
        foreach(Transform trs in infectedSpawn)
        {
            Debug.Log("INFECTED " + count + " -> position : " + trs.position + "    rotation : " + trs.rotation);
            ++count;
        }

        count = 0;
        foreach (Transform trs in survivorSpawn)
        {
            Debug.Log("SURVIVOR " + count + " -> position : " + trs.position + "    rotation : " + trs.rotation);
            ++count;
        }*/

        // Manually simutale physics
        Physics.simulationMode = SimulationMode.Script;

        SCORef.Game = this;
        SCORef.GameData.state = GameData.GameState.WAITING_GAME_START;

        m_ownPlayer = SCORef.GameData.players.Find((Player player) => { return player.Index == SCORef.GameData.ownPlayerIndex; });

        foreach (Player player in SCORef.GameData.players)
        {
            Debug.Log(player.Name + " is infected : " + player.isInfected);

            if (player.Index == SCORef.GameData.ownPlayerIndex)
            {
                Debug.Log("OwnPlayer");

                player.carController = ownCarController;
                if (player.isInfected)
                {
                    ++m_infectedCount;
                    ownCarController.transform.position = infectedSpawn[player.slotId].position;
                    ownCarController.transform.rotation = infectedSpawn[player.slotId].rotation;
                    ownCarController.InitController(true, player.Name);
                }
                else
                {
                    ++m_survivorCount;
                    ownCarController.transform.position = survivorSpawn[player.slotId].position;
                    ownCarController.transform.rotation = survivorSpawn[player.slotId].rotation;
                    ownCarController.InitController(false, player.Name);
                }
            }
            else
            {
                if(player.isInfected)
                {
                    ++m_infectedCount;
                    GameObject car = Instantiate<GameObject>(clientCarPrefab, infectedSpawn[player.slotId].position, infectedSpawn[player.slotId].rotation);
                    player.carController = car.GetComponent<CarController>();
                    player.carController.InitController(true, player.Name);
                }
                else
                {
                    ++m_survivorCount;
                    GameObject car = Instantiate<GameObject>(clientCarPrefab, survivorSpawn[player.slotId].position, survivorSpawn[player.slotId].rotation);
                    player.carController = car.GetComponent<CarController>();
                    player.carController.InitController(false, player.Name);
                }
            }
        }

        survivorCountTxt.text = "SURVIVOR - " + m_survivorCount.ToString("D2");
        infectedCountTxt.text = "INFECTED - " + m_infectedCount.ToString("D2");
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

                //bool performReconciliation = false;

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
                    /*if (DoesNeedReconciliation(predictedInput.physicState, packet.localPhysicState))
                        performReconciliation = true;

                    if (!performReconciliation)
                    {
                        foreach(PlayersStatePacket.PlayerState b in packet.otherPlayerState)
                        {
                            if(predictedInput.others.TryGetValue(b.playerIndex, out PhysicState state) && DoesNeedReconciliation(state, b.physicState))
                            {
                                performReconciliation = true;
                                break;
                            }
                        }
                    }*/
                }

                if(count > 0)
                    m_predictedState.RemoveRange(0, count);

                Debug.Log(m_predictedState.Count);

                // if (performReconciliation)
                {
                    // teleport
                    ownCarController.gameObject.transform.position = packet.localPhysicState.position;
                    ownCarController.gameObject.transform.rotation = packet.localPhysicState.rotation;

                    if (packet.localAtRest)
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
                        ownCarController.CarRb.velocity = packet.localPhysicState.linearVelocity;
                        ownCarController.CarRb.angularVelocity = packet.localPhysicState.angularVelocity;

                        ownCarController.FrontLeftWheelVelocity = packet.localPhysicState.frontLeftWheelVelocity;
                        ownCarController.FrontRightWheelVelocity = packet.localPhysicState.frontRightWheelVelocity;
                        ownCarController.RearLeftWheelVelocity = packet.localPhysicState.rearLeftWheelVelocity;
                        ownCarController.RearRightWheelVelocity = packet.localPhysicState.rearRightWheelVelocity;
                    }

                    foreach(PlayersStatePacket.PlayerState playerState in packet.otherPlayerState)
                    {
                        Player player = SCORef.GameData.players.Find((Player other) => { return other.Index == playerState.playerIndex; });
                        if (player != null)
                        {
                            player.carController.transform.position = playerState.physicState.position;
                            player.carController.transform.rotation = playerState.physicState.rotation;

                            if (playerState.atRest)
                            {
                                player.carController.CarRb.velocity = Vector3.zero;
                                player.carController.CarRb.angularVelocity = Vector3.zero;
                            }
                            else
                            {
                                player.carController.CarRb.velocity = playerState.physicState.linearVelocity;
                                player.carController.CarRb.angularVelocity = playerState.physicState.angularVelocity;

                                player.carController.FrontLeftWheelVelocity = playerState.physicState.frontLeftWheelVelocity;
                                player.carController.FrontRightWheelVelocity = playerState.physicState.frontRightWheelVelocity;
                                player.carController.RearLeftWheelVelocity = playerState.physicState.rearLeftWheelVelocity;
                                player.carController.RearRightWheelVelocity = playerState.physicState.rearRightWheelVelocity;
                            }
                        }
                    }

                    /*for (int i = 0; i < m_predictedState.Count; i++)
                    {
                        foreach (Player player in SCORef.GameData.players)
                        {
                            if (player.carController == null)
                                continue;

                            if (player.Index == SCORef.GameData.ownPlayerIndex)
                                player.carController.SetCarInput(m_predictedState[i].localInput);
                            else
                            {
                                PlayersStatePacket.PlayerState playerState = packet.otherPlayerState.Find((PlayersStatePacket.PlayerState state) => { return state.playerIndex == player.Index; });
                                player.carController.SetCarInput(playerState.inputs);
                            }

                            player.carController.UpdatePhysics();
                        }

                        Physics.Simulate(Time.fixedDeltaTime);
                    }*/
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

                predictedInput.others.Add(player.Index, otherState);
            }

            m_predictedState.Add(predictedInput);
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

                    if (targetPlayer.isInfected)
                    {
                        --m_infectedCount;
                        infectedCountTxt.text = "INFECTED - " + m_infectedCount.ToString("D2");
                    }
                    else
                    {
                        --m_survivorCount;
                        survivorCountTxt.text = "SURVIVOR - " + m_survivorCount.ToString("D2");
                    }

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

                        ++m_infectedCount;
                        --m_survivorCount;
                        infectedCountTxt.text = "INFECTED - " + m_infectedCount.ToString("D2");
                        survivorCountTxt.text = "SURVIVOR - " + m_survivorCount.ToString("D2");
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

    private bool DoesNeedReconciliation(PhysicState predictedState, PhysicState serverState)
    {
        if ((predictedState.position - serverState.position).magnitude > 0.1f)
            return true;

        if ((predictedState.rotation.eulerAngles - serverState.rotation.eulerAngles).magnitude > 0.1f)
            return true;

        if ((predictedState.linearVelocity - serverState.linearVelocity).magnitude > 0.1f)
            return true;

        if ((predictedState.angularVelocity - serverState.angularVelocity).magnitude > 0.1f)
            return true;

        return false;
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
