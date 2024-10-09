using UnityEngine;
using UnityEngine.InputSystem;

public class PlayerInputManager : MonoBehaviour
{
    public PlayerInputAction playerAction { get; private set; } = null;

    [SerializeField]
    private CarController carController;
    [SerializeField]
    private LookController lookController;


    // Action Map
    private void Awake()
    {
        playerAction = new PlayerInputAction();
        EnableCarMap();

        // look functions
        playerAction.PlayerCarMap.LookBack.performed += OnLookBack;
        playerAction.PlayerCarMap.LookBack.canceled += OnLookBack;

        // car functions
        playerAction.PlayerCarMap.Acceleration.performed += OnCarAccelerate;
        playerAction.PlayerCarMap.Acceleration.canceled += OnCarAccelerate;
        playerAction.PlayerCarMap.Turn.performed += OnCarTurn;
        playerAction.PlayerCarMap.Turn.canceled += OnCarTurn;
        playerAction.PlayerCarMap.Break.performed += OnCarBreak;
        playerAction.PlayerCarMap.Break.canceled += OnCarBreak;
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
        playerAction.PlayerCarMap.Break.performed -= OnCarBreak;
        playerAction.PlayerCarMap.Break.canceled -= OnCarBreak;
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
        carController.AccelerationInput = context.ReadValue<float>();
    }

    public void OnCarTurn(InputAction.CallbackContext context)
    {
        carController.SetDesireTurnAngle(context.ReadValue<float>());
    }

    public void OnCarBreak(InputAction.CallbackContext context)
    {
        carController.NeedToBreak = context.ReadValue<float>() > 0.5f;
    }

    public void OnCarRecover(InputAction.CallbackContext context)
    {
        carController.SoftRecover();
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
