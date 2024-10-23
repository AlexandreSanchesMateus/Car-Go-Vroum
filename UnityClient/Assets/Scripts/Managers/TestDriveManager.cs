using TMPro;
using UnityEngine;
using UnityEngine.InputSystem.XR;
using UnityEngine.SceneManagement;

public class TestDriveManager : MonoBehaviour
{
    [SerializeField]
    private CarController carController;
    [SerializeField]
    private TextMeshProUGUI speed;
    [SerializeField]
    private PlayerInputManager inputManager;

    private void Awake()
    {
        Physics.simulationMode = SimulationMode.Script;
    }

    private void Update()
    {
        if (speed)
            speed.text = carController.ShowSpeed.ToString();
    }

    private void FixedUpdate()
    {
        carController.UpdatePhysics();
        Physics.Simulate(Time.fixedDeltaTime);
    }

    public void GoToMenu()
    {
        SceneManager.LoadScene(0);
    }
}
