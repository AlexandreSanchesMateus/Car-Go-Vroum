using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PredictedInput
{
    public UInt16 inputIndex;
    public PlayerInput localInput = new PlayerInput();
    public PhysicState physicState = new PhysicState();

    public Dictionary<int, PhysicState> others = new Dictionary<int, PhysicState>();
}
