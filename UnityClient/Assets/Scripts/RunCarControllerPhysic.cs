using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RunCarControllerPhysic : MonoBehaviour
{
    [SerializeField]
    private CarController controller;

    private void Awake()
    {
        Physics.simulationMode = SimulationMode.Script;
    }

    private void FixedUpdate()
    {
        controller.UpdatePhysics();
        Physics.Simulate(Time.fixedDeltaTime);
    }
}
