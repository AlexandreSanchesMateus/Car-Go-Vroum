using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PhysicState
{
    public Vector3 position;
    public Quaternion rotation;

    public Vector3 linearVelocity;
    public Vector3 angularVelocity;

    public float frontLeftWheelVelocity;
    public float frontRightWheelVelocity;
    public float rearLeftWheelVelocity;
    public float rearRightWheelVelocity;
}