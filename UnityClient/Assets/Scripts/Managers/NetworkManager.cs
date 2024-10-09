using ENet6;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

public class NetworkManager : MonoBehaviour
{
    [SerializeField]
    private GameRefSCO SCORef;

    private ENet6.Host enetHost = null;
    private ENet6.Peer? serverPeer = null;

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

        // On laisse la connexion se faire pendant un maximum de 50 * 100ms = 5s
        for (uint i = 0; i < 50; ++i)
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

        IsConnected = true;
        return true;
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
        }
        else
        {
            Destroy(gameObject);
            Debug.LogWarning("NetworkManager destroyed. There are already an instance or SCORef is null");
        }
    }

    private void OnApplicationQuit()
    {
        ENet6.Library.Deinitialize();
    }

    // FixedUpdate est appelé à chaque Tick (réglé dans le projet)
    void FixedUpdate()
    {
        if (!IsConnected)
            return;

        RunNetwork();
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
                        Debug.Log("?");
                        break;

                    case ENet6.EventType.Connect:
                        Debug.Log("Connect");
                        break;

                    case ENet6.EventType.Disconnect:
                        Debug.Log("Disconnect");
                        serverPeer = null;
                        IsConnected = false;
                        return false;

                    case ENet6.EventType.Receive:
                        // On a reçu un message ! Traitons-le
                        byte[] array = new byte[evt.Packet.Length];
                        Marshal.Copy(evt.Packet.Data, array, 0, evt.Packet.Length);

                        // On gère le message qu'on a reçu
                        HandleMessage(array, evt.Packet.Length);

                        // On n'oublie pas de libérer le packet
                        evt.Packet.Dispose();
                        break;

                    case ENet6.EventType.Timeout:
                        Debug.Log("Timeout");
                        break;
                }
            }
            while (enetHost.CheckEvents(out evt) > 0);
        }

        return true;
    }

    private void HandleMessage(byte[] payload, int size)
    {
        // int offset = 0;
    }
}
