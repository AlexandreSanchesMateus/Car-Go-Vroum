using NetworkProtocol;
using System;
using UnityEngine;

public class PredictedState
{
    public UInt16 inputIndex;

    public Vector3 position;
    public Quaternion rotation;

    public Vector3 linearVelocity;
    public Vector3 angularVelocity;

    public float frontLeftWheelVelocity;
    public float frontRightWheelVelocity;
    public float rearLeftWheelVelocity;
    public float rearRightWheelVelocity;

    public bool IsEqual(PlayersStatePacket.PlayerState state)
    {
        throw new NotImplementedException();
    }
}
