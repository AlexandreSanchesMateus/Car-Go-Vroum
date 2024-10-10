using NaughtyAttributes;
using NetworkProtocol;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

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

    [SerializeField, BoxGroup("Connection")]
    private GameObject connectionPanel;

    [SerializeField, BoxGroup("Lobby")]
    private GameObject lobbyPanel;
    [SerializeField, BoxGroup("Lobby")]
    private GameObject slotPrefab;
    [SerializeField, BoxGroup("Lobby")]
    private Transform content;

    private List<LobbySlot> m_lobbySlots = new List<LobbySlot>();

    private void Awake()
    {
        SCORef.Menu = this;
    }

    public void TryConnectToNetwork()
    {
        // Vérification des entrées
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

        // Tentative de connection
        if (SCORef.network.Connect(ipInputField.text))
        {
            lobbyPanel.SetActive(true);
        }
        else
        {
            connectionError.text = "Connection failed";
            joinPanel.SetActive(true);
        }

        connectionPanel.SetActive(false);
    }

    public void HandleMessage(byte[] message, GameData gameData)
    {
        List<byte> byteArray = new List<byte>(message);

        int offset = 0;
        EOpcode opcode = (EOpcode)Serializer.Deserialize_uByte(byteArray, ref offset);
        switch (opcode)
        {
            case EOpcode.S_GameData:
                gameData.ownPlayerIndex = Serializer.Deserialize_u16(byteArray, ref offset);
                break;

            case EOpcode.S_PlayerList:
                PlayerListPacket packet = PlayerListPacket.Deserialize(byteArray, ref offset);

                break;
        }
    }
}
