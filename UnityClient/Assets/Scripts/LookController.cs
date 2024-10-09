using UnityEngine;
using NaughtyAttributes;
using Cinemachine;
using System.Collections.Generic;
using UnityEngine.Windows;

public class LookController : MonoBehaviour
{
    [SerializeField, BoxGroup("Init")]
    private CinemachineVirtualCamera frontVrCam;
    [SerializeField, BoxGroup("Init")]
    private CinemachineVirtualCamera backVrCam;

    bool m_isLookingBack = false;

    public void ActiveBackVrCam()
    {
        if (m_isLookingBack)
            return;

        frontVrCam.gameObject.SetActive(false);
        backVrCam.gameObject.SetActive(true);
        m_isLookingBack = true;
    }

    public void ActiveFrontVrCam()
    {
        if (!m_isLookingBack)
            return;

        frontVrCam.gameObject.SetActive(true);
        backVrCam.gameObject.SetActive(false);
        m_isLookingBack = false;
    }
}
