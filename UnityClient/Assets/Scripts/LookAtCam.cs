using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;

[RequireComponent(typeof(RectTransform))]
public class LookAtCam : MonoBehaviour
{
    private Transform m_camera;
    private RectTransform m_rectTransform;

    public float initialDist = 5f;

    void Start()
    {
        m_camera = Camera.main.transform;
        m_rectTransform = GetComponent<RectTransform>();
    }

    void Update()
    {
        if (m_camera != null)
        {
            Vector3 dir = m_rectTransform.position - m_camera.position;
            m_rectTransform.rotation = Quaternion.LookRotation(dir, Vector3.up);
        }

        float dist = Vector3.Distance(m_rectTransform.position, m_camera.position);
        m_rectTransform.localScale = Vector3.one * dist / initialDist;
    }
}
