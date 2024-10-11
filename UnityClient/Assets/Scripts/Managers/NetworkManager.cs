using ENet6;
using NetworkProtocol;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Runtime.InteropServices;
using UnityEngine;
using static UnityEngine.Rendering.DebugUI;

public class NetworkManager : MonoBehaviour
{
    [SerializeField]
    private GameRefSCO SCORef;

    private ENet6.Host enetHost = null;
    private ENet6.Peer? serverPeer = null;

    private GameData m_gameData = new GameData();

    public static bool IsConnected { get; private set; } = false;

    public bool Connect(string addressString)
    {
        ENet6.Address address = new ENet6.Address();
        if (!address.SetHost(ENet6.AddressType.Any, addressString))
        {
            Debug.LogError("failed to resolve \"" + addressString + "\"");
            return false;
        }

        Debug.Log("connecting to " + address.GetIP());

        address.Port = 14769;

        // On recréé l'host à la connexion pour l'avoir en IPv4 / IPv6 selon l'adresse
        if (enetHost != null)
            enetHost.Dispose();

        enetHost = new ENet6.Host();
        enetHost.Create(address.Type, 1, 0);
        serverPeer = enetHost.Connect(address, 0);

        // On laisse la connexion se faire pendant un maximum de 40 * 100ms = 4s
        for (uint i = 0; i < 40; ++i)
        {
            ENet6.Event evt = new ENet6.Event();
            if (enetHost.Service(100, out evt) > 0)
            {
                // Nous avons un événement, la connexion a soit pu s'effectuer (ENET_EVENT_TYPE_CONNECT) soit échoué (ENET_EVENT_TYPE_DISCONNECT)
                break; //< On sort de la boucle
            }
        }

        if (serverPeer.Value.State != PeerState.Connected)
        {
            Debug.LogError("connection to \"" + addressString + "\" failed");
            return false;
        }

        Debug.Log("connection successful");

        IsConnected = true;
        return true;
    }

    public void Disconnect()
    {
        if (IsConnected && serverPeer != null)
            serverPeer.Value.Disconnect(0);
    }

    // Start is called before the first frame update
    void Awake()
    {
        if (SCORef && !SCORef.Network)
        {
            SCORef.Network = this;
            DontDestroyOnLoad(this);

            if (!ENet6.Library.Initialize())
                throw new Exception("Failed to initialize ENet");

            m_gameData.state = GameData.GameState.LOBBY;
        }
        else
        {
            Destroy(gameObject);
            Debug.LogWarning("NetworkManager destroyed. There are already an instance or SCORef is null");
        }
    }

    private void OnApplicationQuit()
    {
        if (IsConnected && serverPeer != null)
        {
            serverPeer.Value.DisconnectNow(0);
            enetHost.Flush();
            enetHost.Dispose();
        }

        ENet6.Library.Deinitialize();
    }

    // FixedUpdate est appelé à chaque Tick (réglé dans le projet)
    void FixedUpdate()
    {
        if (IsConnected)
        {
            if (!RunNetwork())
            {
                serverPeer.Value.DisconnectNow(0);
                enetHost.Flush();
                enetHost.Dispose();

                enetHost = null;
                serverPeer = null;
                IsConnected = false;
            }
        }
    }

    private bool RunNetwork()
    {
        ENet6.Event evt = new ENet6.Event();
        if (enetHost.Service(0, out evt) > 0)
        {
            do
            {
                switch (evt.Type)
                {
                    case ENet6.EventType.None:
                    case ENet6.EventType.Connect:
                        Debug.Log("Unexpected event");
                        break;

                    case ENet6.EventType.Disconnect:
                    case ENet6.EventType.Timeout:
                        {
                            Debug.Log("Disconnected from server");
                            if (m_gameData != null && SCORef != null)
                            {
                                switch (m_gameData.state)
                                {
                                    case GameData.GameState.LOBBY:
                                        if (SCORef.Menu != null)
                                            SCORef.Menu.HandleDisconnection(evt.Data);
                                        break;

                                    case GameData.GameState.WAITING_GAME_START:
                                    case GameData.GameState.GAME_STARTED:
                                    case GameData.GameState.GAME_FINISHED:
                                        if (SCORef.Game != null)
                                            SCORef.Game.HandleDisconnection(evt.Data);
                                        break;

                                }
                            }
                        }
                        return false;

                    case ENet6.EventType.Receive:
                        {
                            // On a reçu un message ! Traitons-le
                            byte[] array = new byte[evt.Packet.Length];
                            Marshal.Copy(evt.Packet.Data, array, 0, evt.Packet.Length);

                            if (m_gameData != null && SCORef != null)
                            {
                                switch (m_gameData.state)
                                {
                                    case GameData.GameState.LOBBY:
                                        if (SCORef.Menu != null)
                                            SCORef.Menu.HandleMessage(array, m_gameData);
                                        break;

                                    case GameData.GameState.WAITING_GAME_START:
                                    case GameData.GameState.GAME_STARTED:
                                    case GameData.GameState.GAME_FINISHED:
                                        if (SCORef.Game != null)
                                            SCORef.Game.HandleMessage(array, m_gameData);
                                        break;

                                }
                            }

                            // On n'oublie pas de libérer le packet
                            evt.Packet.Dispose();
                        }
                        break;
                }
            }
            while (enetHost.CheckEvents(out evt) > 0);
        }

        return true;
    }

    public bool SendToNetwork(byte channelID, ref Packet packet)
    {
        if(IsConnected && serverPeer != null)
            return serverPeer.Value.Send(channelID, ref packet);

        return false;
    }

    public bool BuildAndSendPacketToNetwork<T>(T packet, PacketFlags flags, byte channelID) where T : BasePacket
    {
        Packet enetPacket = BuildPacket<T>(packet, flags);
        return SendToNetwork(channelID, ref enetPacket);
    }

    public static Packet BuildPacket<T>(T packet, PacketFlags flags) where T : BasePacket
    {
        Packet enetPacket = new Packet();
        List<byte> byteArray = new List<byte>();

        Serializer.Serialize_uByte(byteArray, (byte)packet.Opcode);
        packet.Serialize(byteArray);

        enetPacket.Create(byteArray.ToArray(), byteArray.Count, flags);
        return enetPacket;
    }
}
