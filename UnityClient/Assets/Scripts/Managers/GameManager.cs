using NetworkProtocol;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class GameManager : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    public void HandleMessage(byte[] message, GameData gameData)
    {
        List<byte> byteArray = new List<byte>(message);

        int offset = 0;
        EOpcode opcode = (EOpcode)Serializer.Deserialize_uByte(byteArray, ref offset);
        switch (opcode)
        {

        }
    }
}
