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

    public void InitSlot(int id, string psedo, bool ready)
    {
        idTxt.text = id.ToString();
        psedoTxt.text = psedo;
        ChnageReadyStatus(ready);
    }

    void ChnageReadyStatus(bool ready)
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
