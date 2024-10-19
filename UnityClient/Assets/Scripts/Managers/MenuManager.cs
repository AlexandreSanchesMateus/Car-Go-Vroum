using ENet6;
using NaughtyAttributes;
using NetworkProtocol;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class MenuManager : MonoBehaviour
{
    [SerializeField, BoxGroup("Init")]
    private GameRefSCO SCORef;

    [SerializeField, BoxGroup("Join")]
    private GameObject joinPanel;
    [SerializeField, BoxGroup("Join")]
    private TextMeshProUGUI connectionError;
    [SerializeField, BoxGroup("Join")]
    private TMP_InputField ipInputField;
    [SerializeField, BoxGroup("Join")]
    private TMP_InputField pseudoInputField;
    [SerializeField, BoxGroup("Join")]
    private Button joinBtn;

    [SerializeField, BoxGroup("Connection")]
    private GameObject connectionPanel;

    [SerializeField, BoxGroup("Lobby")]
    private GameObject lobbyPanel;
    [SerializeField, BoxGroup("Lobby")]
    private GameObject slotPrefab;
    [SerializeField, BoxGroup("Lobby")]
    private Transform content;
    [SerializeField, BoxGroup("Lobby")]
    private Button readyBtn;
    [SerializeField, BoxGroup("Lobby")]
    private TextMeshProUGUI readyTxt;

    private Dictionary<int, LobbySlot> m_lobbySlots = new Dictionary<int, LobbySlot>();
    private bool m_lastReadyStatus = false;

    private void Awake()
    {
        SCORef.Menu = this;
        SCORef.GameData.state = GameData.GameState.LOBBY;

        // vérifier s'il est déjç connecté (revernir après une partie)
        if (SCORef.Network.IsConnected)
        {
            joinPanel.SetActive(false);
            lobbyPanel.SetActive(true);

            foreach (Player player in SCORef.GameData.players)
            {
                player.ready = false;

                LobbySlot component = Instantiate<GameObject>(slotPrefab, content).GetComponent<LobbySlot>();
                component.InitSlot(player);

                m_lobbySlots.Add(player.Index, component);
            }
        }

        joinBtn.interactable = false;

        pseudoInputField.onSubmit.AddListener(this.TryConnectToNetwork);
        ipInputField.onSubmit.AddListener(this.TryConnectToNetwork);

        pseudoInputField.onValueChanged.AddListener(this.UpdateJoinBtnInteractivity);
        ipInputField.onValueChanged.AddListener(this.UpdateJoinBtnInteractivity);
    }

    private void UpdateJoinBtnInteractivity(string nan)
    {
        if(string.IsNullOrEmpty(ipInputField.text) || string.IsNullOrEmpty(pseudoInputField.text))
            joinBtn.interactable = false;
        else
            joinBtn.interactable = true;
    }

    public void TryConnectToNetwork(string nan)
    {
        // VÃ©rification des entrÃ©es
        if (string.IsNullOrEmpty(ipInputField.text))
        {
            connectionError.text = "Enter a valid IP adress";
            return;
        }
        else if (string.IsNullOrEmpty(pseudoInputField.text))
        {
            connectionError.text = "Enter a valid nickname";
            return;
        }

        joinPanel.SetActive(false);
        connectionPanel.SetActive(true);

        StartCoroutine(ConnectToNetwork());
    }

    public void SendReady()
    {
        PlayerReadyPacket readyPacket = new PlayerReadyPacket();
        readyPacket.ready = !m_lastReadyStatus;

        SCORef.Network.BuildAndSendPacketToNetwork<PlayerReadyPacket>(readyPacket, ENet6.PacketFlags.Reliable, 0);
        readyBtn.interactable = false;
    }

    public void HandleMessage(byte[] message)
    {
        List<byte> byteArray = new List<byte>(message);

        int offset = 0;
        EOpcode opcode = (EOpcode)Serializer.Deserialize_uByte(byteArray, ref offset);
        switch (opcode)
        {
            case EOpcode.S_GameData:
                {
                    connectionPanel.SetActive(false);
                    lobbyPanel.SetActive(true);

                    GameDataPacket packet = GameDataPacket.Deserialize(byteArray, ref offset);
                    SCORef.GameData.ownPlayerIndex = packet.targetPlayerIndex;

                    foreach (GameDataPacket.PlayerPacketData data in packet.playerList)
                    {
                        Player player = new Player(data.index, data.name);
                        player.ready = data.ready;
                        SCORef.GameData.players.Add(player);

                        LobbySlot component = Instantiate<GameObject>(slotPrefab, content).GetComponent<LobbySlot>();
                        component.InitSlot(player);

                        m_lobbySlots.Add(data.index, component);
                    }
                }
                break;

            case EOpcode.S_PlayerConnected:
                {
                    PlayerConnectPacket packet = PlayerConnectPacket.Deserialize(byteArray, ref offset);

                    Player player = new Player(packet.playerIndex, packet.name);
                    player.ready = packet.ready;
                    SCORef.GameData.players.Add(player);

                    LobbySlot component = Instantiate<GameObject>(slotPrefab, content).GetComponent<LobbySlot>();
                    component.InitSlot(player);

                    m_lobbySlots.Add(packet.playerIndex, component);
                }
                break;

            case EOpcode.S_PlayerDisconnected:
                {
                    PlayerDisconnectedPacket packet = PlayerDisconnectedPacket.Deserialize(byteArray,ref offset);

                    Player targetPlayer = SCORef.GameData.players.Find((Player other) => { return other.Index == packet.playerIndex; });

                    if (targetPlayer != null)
                        SCORef.GameData.players.Remove(targetPlayer);
                    else
                        Debug.LogWarning("Can't find disconnected player");

                    if (m_lobbySlots.TryGetValue(packet.playerIndex, out LobbySlot other))
                    {
                        Destroy(other.gameObject);
                        m_lobbySlots.Remove(packet.playerIndex);
                    }
                }
                break;

            case EOpcode.S_Ready:
                {
                    ReadyPacket readyPacket = ReadyPacket.Deserialize(byteArray, ref offset);

                    Player player = SCORef.GameData.players.Find((Player other) => { return other.Index == readyPacket.playerIndex; });
                    if (player != null)
                    {
                        player.ready = readyPacket.ready;

                        if (SCORef.GameData.ownPlayerIndex == readyPacket.playerIndex)
                        {
                            m_lastReadyStatus = readyPacket.ready;
                            readyBtn.interactable = true;

                            if (m_lastReadyStatus)
                                readyTxt.text = "NOT READY";
                            else
                                readyTxt.text = "READY";
                        }

                        if (m_lobbySlots.TryGetValue(readyPacket.playerIndex, out LobbySlot other))
                            other.ChangeReadyStatus(m_lastReadyStatus);
                    }
                    else
                        Debug.LogWarning("Can't find disconnected player");

                }
                break;

            case EOpcode.S_RunningState:
                GameStateRunningPacket gameStateRunningPacket = GameStateRunningPacket.Deserialize(byteArray, ref offset);

                foreach(GameStateRunningPacket.RunningPacketData other in gameStateRunningPacket.playerList)
                {
                    Player player = SCORef.GameData.players.Find((Player player) => { return player.Index == other.playerIndex; });
                    if(player != null)
                    {
                        player.isInfected = other.isInfected;
                        player.slotId = other.slotId;
                    }
                    else
                        Debug.LogWarning("Can't find player for running state");
                }

                // Load game scene
                SceneManager.LoadScene(1);
                break;
        }
    }

    public void HandleDisconnection(uint disconectId)
    {
        SCORef.GameData.players.Clear();
        SCORef.GameData.ownPlayerIndex = -1;

        m_lastReadyStatus = false;
        readyBtn.interactable = true;
        readyTxt.text = "READY";

        foreach (var packet in m_lobbySlots)
        {
            Destroy(packet.Value.gameObject);
        }
        m_lobbySlots.Clear();

        connectionPanel.SetActive(false);
        lobbyPanel.SetActive(false);
        joinPanel.SetActive(true);

        switch (disconectId)
        {
            case (uint)DisconnectReport.DISCONNECTED:
                connectionError.text = "";
                break;

            case (uint)DisconnectReport.SERVER_END:
                connectionError.text = "The lobby has closed";
                break;

            case (uint)DisconnectReport.KICK:
                connectionError.text = "You have been kicked from his lobby";
                break;

            case (uint)DisconnectReport.LOBBY_FULL:
                connectionError.text = "Lobby full";
                break;

            case (uint)DisconnectReport.GAME_LAUNCHED:
                connectionError.text = "Game already started";
                break;
        }
    }

    private IEnumerator ConnectToNetwork()
    {
        yield return null;
        if (SCORef.Network.Connect(ipInputField.text))
        {
            PlayerNamePacket namePacket = new PlayerNamePacket();
            namePacket.name = pseudoInputField.text;
            SCORef.Network.BuildAndSendPacketToNetwork<PlayerNamePacket>(namePacket, ENet6.PacketFlags.Reliable, 0);
        }
        else
        {
            connectionError.text = "Connection failed - Lobby full or invalid ip address";
            connectionPanel.SetActive(false);
            joinPanel.SetActive(true);
        }
    }

    public void LeaveLobby()
    {
        if(SCORef != null && SCORef.Network != null)
            SCORef.Network.Disconnect();
    }

    public void ReturnToDesktop()
    {
        Application.Quit();
    }
}
