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

    [SerializeField, BoxGroup("Settings")]
    private float sensibility = 0.5f;
    [SerializeField, BoxGroup("Settings")]
    private float speed;
    [SerializeField, Range(-0.5f, 1.5f), BoxGroup("Settings")]
    private float maxXScreen;
    [SerializeField, Range(-0.5f, 1.5f), BoxGroup("Settings")]
    private float minXScreen;
    [SerializeField, Range(-0.5f, 1.5f), BoxGroup("Settings")]
    private float maxYScreen;
    [SerializeField, Range(-0.5f, 1.5f), BoxGroup("Settings")]
    private float minYScreen;

    private bool m_isLookingBack = false;
    private CinemachineComposer m_frontComposer;
    private CinemachineComposer m_backComposer;

    private float m_XTarget = 0.5f;
    private float m_yTarget = 0.5f;

    public float XTarget { set {
            m_XTarget = Mathf.Clamp(value * sensibility, minXScreen, maxXScreen);
        } }


    public void Awake()
    {
        m_frontComposer = frontVrCam.GetCinemachineComponent<CinemachineComposer>();
        m_backComposer = backVrCam.GetCinemachineComponent<CinemachineComposer>();
    }

    public void Update()
    {
       /* if (m_isLookingBack)
        {
            //m_backComposer.m_ScreenX = Mathf.Lerp(m_backComposer.m_ScreenX, m_XTarget, speed * Time.deltaTime);
        }
        else
        {
            m_frontComposer.m_ScreenX = Mathf.Lerp(m_backComposer.m_ScreenX, m_XTarget, speed * Time.deltaTime);
        }*/
    }

    public void ActiveBackVrCam()
    {
        if (m_isLookingBack)
            return;

        frontVrCam.gameObject.SetActive(false);
        m_frontComposer.m_ScreenX = 0.5f;
        m_frontComposer.m_ScreenY = 0.5f;
        backVrCam.gameObject.SetActive(true);

        m_XTarget = 0.5f;
        m_yTarget = 0.5f;
        m_isLookingBack = true;
}

    public void ActiveFrontVrCam()
    {
        if (!m_isLookingBack)
            return;

        frontVrCam.gameObject.SetActive(true);
        backVrCam.gameObject.SetActive(false);
        m_backComposer.m_ScreenX = 0.5f;
        m_backComposer.m_ScreenY = 0.5f;

        m_XTarget = 0.5f;
        m_yTarget = 0.5f;
        m_isLookingBack = false;
    }
}
