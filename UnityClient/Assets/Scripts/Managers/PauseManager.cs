using NaughtyAttributes;
using UnityEngine;
using UnityEngine.InputSystem;
using UnityEngine.SceneManagement;

public class PauseManager : MonoBehaviour
{
    [SerializeField, BoxGroup("Init")]
    private GameRefSCO refSCO;
    [SerializeField, BoxGroup("Init")]
    private PlayerInputManager inputManager;
    [SerializeField, BoxGroup("Init")]
    private GameObject parentCanvas;


    private void Start()
    {
        inputManager.playerAction.UI.CloseMenu.performed += CloseMenu;
        inputManager.playerAction.PlayerCarMap.OpenMenu.performed += OpenMenu;

        inputManager.DisableUIMap();
        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
    }

    private void OnDisable()
    {
        inputManager.playerAction.UI.CloseMenu.performed -= CloseMenu;
        inputManager.playerAction.PlayerCarMap.OpenMenu.performed -= OpenMenu;
    }

    private void CloseMenu(InputAction.CallbackContext context)
    {
        parentCanvas.SetActive(false);
        inputManager.DisableUIMap();
        inputManager.EnableCarMap();
        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
    }

    private void OpenMenu(InputAction.CallbackContext context)
    {
        parentCanvas.SetActive(true);
        inputManager.DisableCarMap();
        inputManager.EnableUIMap();
        Cursor.lockState = CursorLockMode.Confined;
        Cursor.visible = true;
    }

    public void Leave()
    {
        SceneManager.LoadScene(0);

        if(refSCO != null && refSCO.Network != null)
            refSCO.Network.Disconnect();
    }
}
