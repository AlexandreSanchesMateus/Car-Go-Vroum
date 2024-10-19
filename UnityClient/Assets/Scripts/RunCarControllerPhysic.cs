using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class RunCarControllerPhysic : MonoBehaviour
{
    [SerializeField]
    private CarController controller;
    [SerializeField]
    private TextMeshProUGUI speed;

    private void Awake()
    {
        Physics.simulationMode = SimulationMode.Script;
    }

    private void Update()
    {
        if (speed)
            speed.text = ((int)(Mathf.Abs(Vector3.Dot(controller.transform.forward, controller.CarRb.velocity)) * 3.6f)).ToString();
    }

    private void FixedUpdate()
    {
        controller.UpdatePhysics();
        Physics.Simulate(Time.fixedDeltaTime);
    }
}
