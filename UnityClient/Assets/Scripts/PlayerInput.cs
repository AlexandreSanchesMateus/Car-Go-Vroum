using NetworkProtocol;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerInput
{
    public int acceleration = 0;
    public int steer = 0;
    public bool brake = false;
    public bool softRecover = false;

    public void Serialize(List<byte> byteArray)
    {
        byte inputByte = 0;

        if(acceleration == 1)
            inputByte |= 0b1;
        else if(acceleration == -1)
            inputByte |= 0b10;

        if (steer == 1)
            inputByte |= 0b100;
        else if (steer == -1)
            inputByte |= 0b1000;

        if (brake)
            inputByte |= 0b10000;

        if (softRecover)
            inputByte |= 0b100000;

        Serializer.Serialize_uByte(byteArray, inputByte);
    }

    public static PlayerInput Deserialize(List<byte> byteArray, ref int offset)
    { 
        PlayerInput input = new PlayerInput();
        byte inputByte = Serializer.Deserialize_uByte(byteArray, ref offset);

        if((inputByte & 0b1) != 0)
            input.acceleration = 1;
        else if ((inputByte & 0b10) != 0)
            input.acceleration = -1;

        if ((inputByte & 0b100) != 0)
            input.steer = 1;
        else if ((inputByte & 0b1000) != 0)
            input.steer = -1;

        if ((inputByte & 0b10000) != 0)
            input.brake = true;

        if ((inputByte & 0b100000) != 0)
            input.softRecover = true;

        return input;
    }
}
