using UnityEngine;
using UnityEngine.VFX;

[System.Serializable]
public class WheelData
{
    [SerializeField]
    private Transform wheelPos;
    [SerializeField]
    private GameObject model;
    [SerializeField]
    private VisualEffect visualEffect;

    private float m_heightTarget;
    private bool m_shouldSkidmark = false;
    private Vector3 m_impactPos;
    private Vector3 m_normal;
    private float m_friction;
    private int m_skidID = -1;

    public void UpdateVisual(Rigidbody rb, float interpolationSpeed)
    {
        float wheelSpeed = Vector3.Dot(wheelPos.forward, rb.GetPointVelocity(wheelPos.position));
        model.transform.Rotate(new Vector3(wheelSpeed, 0, 0));

        model.transform.localPosition = Vector3.MoveTowards(model.transform.localPosition, new Vector3(0, m_heightTarget, 0), interpolationSpeed * Time.deltaTime);
    }

    public void UpdateHeight(float dist, float wheelHight)
    {
        // model.transform.localPosition = new Vector3(0, -dist + wheelHight, 0);

        m_heightTarget = -dist + wheelHight;

        if(m_shouldSkidmark)
            visualEffect.transform.localPosition = new Vector3(0, -dist, 0);
    }

    public void UpdateSkidmark(Skidmarks skidmarksController, float startSkidmark)
    {
        if (m_shouldSkidmark)
            m_skidID = skidmarksController.AddSkidMark(m_impactPos, m_normal, Mathf.Clamp01((m_friction - startSkidmark) * 10), m_skidID);
        else
            m_skidID = -1;
    }

    public void StartSkidmark(float friction, Vector3 impact, Vector3 normal)
    {
        m_shouldSkidmark = true;
        m_friction = friction;
        m_impactPos = impact;
        m_normal = normal;
        visualEffect.SendEvent("OnPlay");
    }

    public void StopSkidmark()
    {
        m_shouldSkidmark = false;
        visualEffect.SendEvent("OnStop");
    }

    public Transform GetWheelTrs() { return wheelPos; }

    public GameObject GetModel() { return model; }
}
