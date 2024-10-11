using TMPro;
using UnityEngine;

public class LobbySlot : MonoBehaviour
{
    [SerializeField]
    private TextMeshProUGUI idTxt;
    [SerializeField]
    private TextMeshProUGUI psedoTxt;
    [SerializeField]
    private TextMeshProUGUI statusTxt;
    [SerializeField]
    private Color readyColor;
    [SerializeField]
    private Color notReadyColor;

    public void InitSlot(Player targetPlayer)
    {
        idTxt.text = "#" + targetPlayer.Index.ToString();
        psedoTxt.text = targetPlayer.Name;
        ChangeReadyStatus(targetPlayer.ready);
    }

    public void ChangeReadyStatus(bool ready)
    {
        if(ready)
        {
            statusTxt.text = "READY";
            statusTxt.color = readyColor;
        }
        else
        {
            statusTxt.text = "NOT READY";
            statusTxt.color = notReadyColor;
        }
    }
}
