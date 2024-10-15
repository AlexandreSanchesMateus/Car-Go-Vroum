using UnityEngine;
using UnityEngine.InputSystem;

public class PlayerInputManager : MonoBehaviour
{
    public PlayerInputAction playerAction { get; private set; } = null;

    [SerializeField]
    private CarController carController;
    [SerializeField]
    private LookController lookController;

    [SerializeField]
    private bool activeOnStart = false;

    public PlayerInput m_lastInput { get; private set; }

    // Action Map
    private void Awake()
    {
        playerAction = new PlayerInputAction();
        m_lastInput = new PlayerInput();

        if (activeOnStart)
            EnableCarMap();

        // look functions
        playerAction.PlayerCarMap.LookBack.performed += OnLookBack;
        playerAction.PlayerCarMap.LookBack.canceled += OnLookBack;

        // car functions
        playerAction.PlayerCarMap.Acceleration.performed += OnCarAccelerate;
        playerAction.PlayerCarMap.Acceleration.canceled += OnCarAccelerate;
        playerAction.PlayerCarMap.Turn.performed += OnCarTurn;
        playerAction.PlayerCarMap.Turn.canceled += OnCarTurn;
        playerAction.PlayerCarMap.Break.performed += OnCarBrake;
        playerAction.PlayerCarMap.Break.canceled += OnCarBrake;
        playerAction.PlayerCarMap.Recover.performed += OnCarRecover;
    }

    private void OnDestroy()
    {
        // look functions
        playerAction.PlayerCarMap.LookBack.performed -= OnLookBack;
        playerAction.PlayerCarMap.LookBack.canceled -= OnLookBack;

        // car functions
        playerAction.PlayerCarMap.Acceleration.performed -= OnCarAccelerate;
        playerAction.PlayerCarMap.Acceleration.canceled -= OnCarAccelerate;
        playerAction.PlayerCarMap.Turn.performed -= OnCarTurn;
        playerAction.PlayerCarMap.Turn.canceled -= OnCarTurn;
        playerAction.PlayerCarMap.Break.performed -= OnCarBrake;
        playerAction.PlayerCarMap.Break.canceled -= OnCarBrake;
        playerAction.PlayerCarMap.Recover.performed -= OnCarRecover;

        DisableCarMap();
    }

    public void EnableCarMap()
    {
        if(playerAction != null && !playerAction.PlayerCarMap.enabled)
            playerAction.PlayerCarMap.Enable();
    }

    public void DisableCarMap()
    {
        if (playerAction != null && playerAction.PlayerCarMap.enabled)
            playerAction.PlayerCarMap.Disable();
    }


    // Car Controller
    public void OnCarAccelerate(InputAction.CallbackContext context)
    {
        int acceleration = (int)context.ReadValue<float>();
        m_lastInput.acceleration = acceleration;
        carController.AccelerationInput = acceleration;
    }

    public void OnCarTurn(InputAction.CallbackContext context)
    {
        int turn = (int)context.ReadValue<float>();
        m_lastInput.steer = turn;
        carController.SetDesireTurnAngle(turn);
    }

    public void OnCarBrake(InputAction.CallbackContext context)
    {
        if (context.phase == InputActionPhase.Performed)
        {
            m_lastInput.brake = true;
            carController.NeedToBrake = true;
        }
        else
        {
            m_lastInput.brake = false;
            carController.NeedToBrake = false;
        }
    }

    public void OnCarRecover(InputAction.CallbackContext context)
    {
        if(context.phase == InputActionPhase.Performed)
        {
            m_lastInput.softRecover = true;
            carController.SoftRecover();
        }
        else
            m_lastInput.softRecover = false;
    }


    // Car Look
    public void OnLookBack(InputAction.CallbackContext context)
    {
        if (context.phase == InputActionPhase.Performed)
            lookController.ActiveBackVrCam();
        else
            lookController.ActiveFrontVrCam();
    }
}
