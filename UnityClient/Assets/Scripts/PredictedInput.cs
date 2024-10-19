using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PredictedInput
{
    public UInt16 inputIndex;
    public PlayerInput localInput;

    public PhysicState physicState = new PhysicState();
    public List<PhysicState> others = new List<PhysicState>();
}
