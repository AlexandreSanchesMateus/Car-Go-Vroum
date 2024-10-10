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

    public int m_playerIndex { get; private set; }

    public void InitSlot(int id, string psedo, bool ready)
    {
        m_playerIndex = id;

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
