using UnityEngine;
using System.Collections.Generic;
using UnityEngine.InputSystem;
using NaughtyAttributes;
using TMPro;
using DG.Tweening;
using System.Collections;
using UnityEngine.UIElements;

public class CarController : MonoBehaviour
{
    [SerializeField, BoxGroup("Init"), Required]
    private Rigidbody carRb;

    [SerializeField, BoxGroup("Wheels")]
    private WheelData LF_wheelData;
    [SerializeField, BoxGroup("Wheels")]
    private WheelData RF_wheelData;
    [SerializeField, BoxGroup("Wheels")]
    private WheelData LB_wheelData;
    [SerializeField, BoxGroup("Wheels")]
    private WheelData RB_wheelData;

    [SerializeField, BoxGroup("Wheels")]
    private float wheelHeight;
    [SerializeField, BoxGroup("Wheels")]
    private float wheelInterpolationSpeed;

    [SerializeField, BoxGroup("Suspension Settings")]
    private float restLenght;
    [SerializeField, BoxGroup("Suspension Settings")]
    private float springTravel;
    [SerializeField, BoxGroup("Suspension Settings")]
    private float springStiffness;
    [SerializeField, BoxGroup("Suspension Settings")]
    private float damperStiffness;
    [SerializeField, BoxGroup("Suspension Settings")]
    private LayerMask groundLayer;

    [SerializeField, BoxGroup("Friction Settings")]
    private float tireMass;
    [SerializeField, BoxGroup("Friction Settings"), Range(0, 1)]
    private float frictionStartSkidmark = 0.6f;
    [SerializeField, BoxGroup("Friction Settings")]
    private AnimationCurve tireFriction;

    [SerializeField, BoxGroup("Engine Settings")]
    private float engineTorque;
    [SerializeField, BoxGroup("Engine Settings")]
    private AnimationCurve virtualEngine;
    [SerializeField, BoxGroup("Engine Settings")]
    private float topSpeed;
    [SerializeField, BoxGroup("Engine Settings")]
    private float topReverseSpeed;
    [SerializeField, BoxGroup("Engine Settings")]
    private float softBreak;
    [SerializeField, BoxGroup("Engine Settings")]
    private float hardBreak;

    [SerializeField, BoxGroup("Steering Settings")]
    private float steeringSpeed;
    [SerializeField, BoxGroup("Steering Settings")]
    private float steeringAngle;

    [SerializeField, BoxGroup("Flipping Settings")]
    float flippingForce;
    [SerializeField, BoxGroup("Flipping Settings")]
    private LayerMask flipMask;

    public Rigidbody CarRb { get { return carRb; } }
    public int CarSpeed { get; private set; }

    private float m_maxSpringLenght;
    private int m_wheelGroundedCount;

    private float m_desireTurnAngle;
    private float m_currentTurnAngle;

    private float m_frontRearDistance;
    private float m_rearWheelDistance;

    private Skidmarks m_skidmarksController;
    private PlayerInput m_carInput = new PlayerInput();


    private void Start()
    {
        m_skidmarksController = GameObject.FindGameObjectWithTag("SkidmarkController").GetComponent<Skidmarks>();

        m_maxSpringLenght = restLenght + springTravel + wheelHeight;
        m_frontRearDistance = Mathf.Abs(LB_wheelData.GetWheelTrs().localPosition.z - LF_wheelData.GetWheelTrs().localPosition.z);
        m_rearWheelDistance = Mathf.Abs(LB_wheelData.GetWheelTrs().localPosition.x - RB_wheelData.GetWheelTrs().localPosition.x);

        LF_wheelData.UpdateHeight(restLenght, wheelHeight);
        RF_wheelData.UpdateHeight(restLenght, wheelHeight);
        LB_wheelData.UpdateHeight(restLenght, wheelHeight);
        RB_wheelData.UpdateHeight(restLenght, wheelHeight);
    }

    private void Update()
    {
        // Update speed UI
        CarSpeed = (int)(carRb.velocity.magnitude * 3.6f);

        // Update visual
        LF_wheelData.UpdateVisual(carRb, wheelInterpolationSpeed);
        LF_wheelData.UpdateSkidmark(m_skidmarksController, frictionStartSkidmark);

        RF_wheelData.UpdateVisual(carRb, wheelInterpolationSpeed);
        RF_wheelData.UpdateSkidmark(m_skidmarksController, frictionStartSkidmark);

        LB_wheelData.UpdateVisual(carRb, wheelInterpolationSpeed);
        LB_wheelData.UpdateSkidmark(m_skidmarksController, frictionStartSkidmark);

        RB_wheelData.UpdateVisual(carRb, wheelInterpolationSpeed);
        RB_wheelData.UpdateSkidmark(m_skidmarksController, frictionStartSkidmark);
    }

    public void UpdatePhysics()
    {
        m_currentTurnAngle = Mathf.MoveTowards(m_currentTurnAngle, m_desireTurnAngle, (steeringAngle / steeringSpeed) * Time.fixedDeltaTime);

        Transform RF_Wheel = RF_wheelData.GetWheelTrs();
        Transform LF_Wheel = LF_wheelData.GetWheelTrs();

        if (m_currentTurnAngle != 0f)
        {
            // Angle of the other wheel
            float r = m_frontRearDistance / Mathf.Sin(m_currentTurnAngle * Mathf.Deg2Rad);
            float b = Mathf.Sqrt(Mathf.Pow(r, 2) - Mathf.Pow(m_frontRearDistance, 2));
            float angle = Mathf.Atan(m_frontRearDistance / (m_rearWheelDistance + b)) * Mathf.Rad2Deg;

            if (m_currentTurnAngle > 0f)
            {
                RF_Wheel.transform.localEulerAngles = new Vector3(0f, m_currentTurnAngle, 0f);
                LF_Wheel.transform.localEulerAngles = new Vector3(0f, angle, 0f);
            }
            else if (m_currentTurnAngle < 0f)
            {
                LF_Wheel.transform.localEulerAngles = new Vector3(0f, m_currentTurnAngle, 0f);
                RF_Wheel.transform.localEulerAngles = new Vector3(0f, -angle, 0f);
            }
        }
        else
        {
            // Return to 0;
            LF_Wheel.transform.localEulerAngles = new Vector3(0f, 0f, 0f);
            RF_Wheel.transform.localEulerAngles = new Vector3(0f, 0f, 0f);
        }

        m_wheelGroundedCount = 0;
        // Wheel physic
        UpdateWheelPhysics(LF_wheelData, true);
        UpdateWheelPhysics(RF_wheelData, true);
        UpdateWheelPhysics(LB_wheelData, true);
        UpdateWheelPhysics(RB_wheelData, true);
    }

    void UpdateWheelPhysics(WheelData wheelData, bool driveWheel)
    {
        Transform wheelTransform = wheelData.GetWheelTrs();

        Ray ray = new Ray(wheelTransform.position, -wheelTransform.up);
        if (Physics.Raycast(ray, out RaycastHit hitInfo, m_maxSpringLenght, groundLayer))
        {

#if UNITY_EDITOR
            Debug.DrawRay(wheelTransform.position, transform.rotation * Vector3.down * hitInfo.distance, Color.green);
#endif

            Vector3 wheelVelocity = carRb.GetPointVelocity(wheelTransform.position);

            float forwardVelocity = Vector3.Dot(wheelTransform.forward, wheelVelocity);
            float sideVelocity = Vector3.Dot(wheelTransform.right, wheelVelocity);
            float upVelocity = Vector3.Dot(wheelTransform.up, wheelVelocity);

#if UNITY_EDITOR
            Debug.DrawRay(hitInfo.point, wheelTransform.forward * forwardVelocity, Color.blue);
            Debug.DrawRay(hitInfo.point, wheelTransform.right * sideVelocity, Color.red);
            Debug.DrawRay(hitInfo.point, wheelTransform.up * upVelocity, Color.green);
#endif

            float movingDirection = Mathf.Sign(forwardVelocity);
            if (m_carInput.brake)
            {
                if (Mathf.Abs(forwardVelocity) < 0.3f)
                    carRb.AddForceAtPosition(-wheelTransform.forward * forwardVelocity * 200, hitInfo.point);
                else
                    carRb.AddForceAtPosition(-wheelTransform.forward * movingDirection * hardBreak, hitInfo.point);
            }
            else
            {
                if (m_carInput.acceleration != 0f)
                {
                    if (Mathf.Abs(forwardVelocity) < 5f || (Mathf.Sign(m_carInput.acceleration) == movingDirection))
                    {
                        // Accelerate
                        float normelizeSpeed = Mathf.Clamp01(Mathf.Abs(forwardVelocity * 3.6f) / (m_carInput.acceleration > 0 ? topSpeed : topReverseSpeed));
                        float availableTorque = virtualEngine.Evaluate(normelizeSpeed) * m_carInput.acceleration * engineTorque;
                        carRb.AddForceAtPosition(wheelTransform.forward * availableTorque, hitInfo.point);
                    }
                    else
                    {
                        // Soft breaking
                        carRb.AddForceAtPosition(-movingDirection * wheelTransform.forward * softBreak, hitInfo.point);
                    }
                }
                else
                {
                    // Air drag
                    carRb.AddForceAtPosition(-wheelTransform.forward * forwardVelocity * 5, hitInfo.point);
                }
            }

            // -------------------------- Vertical Force (suspension) --------------------------
            float currentSpringLenght = hitInfo.distance - wheelHeight;
            float springCompression = (restLenght - currentSpringLenght) / springTravel;
            float springForce = (springStiffness * springCompression) - (upVelocity * damperStiffness);
            carRb.AddForceAtPosition(wheelTransform.up * springForce, wheelTransform.position);

            // ------------------------ Horizontal Force (side friction) ------------------------
            float sideSpeed = Mathf.Abs(sideVelocity);
            float sideFrictionRation = sideSpeed / (sideSpeed + Mathf.Abs(forwardVelocity));
            carRb.AddForceAtPosition(tireMass * (-wheelTransform.right * sideVelocity * tireFriction.Evaluate(sideFrictionRation) / Time.fixedDeltaTime), wheelTransform.position - wheelTransform.up * 0.45f);


            // Move Model
            wheelData.UpdateHeight(hitInfo.distance, wheelHeight);
            ++m_wheelGroundedCount;
        }
        else
        {

#if UNITY_EDITOR
            Debug.DrawRay(wheelTransform.position, transform.rotation * Vector3.down * restLenght, Color.red);
#endif

            // Move Model
            wheelData.UpdateHeight(m_maxSpringLenght - wheelHeight, wheelHeight);
            wheelData.StopSkidmark();
        }
    }

    public void SetCarInput(PlayerInput newInput)
    {
        m_carInput = newInput;
        m_desireTurnAngle = newInput.steer * steeringAngle;
    }

    public void SetTurnAngle(float angle)
    {
        m_currentTurnAngle = angle;
    }

    public void InitController(bool infected, string name)
    {
        /*if (nameTxt != null)
            nameTxt.text = name;*/

       /* if (infected)
            SetInfectedModel();*/
    }

    private void OnCollisionStay(Collision collision)
    {
        if (collision != null && ((1 << collision.gameObject.layer) & flipMask) != 0 && m_wheelGroundedCount < 2)
        {
            // Flipping force (torque)
            Vector3 surfaceNormal = collision.GetContact(0).normal;
            float angle = Vector3.SignedAngle(surfaceNormal, transform.up, transform.forward);
            carRb.AddTorque(transform.forward * -flippingForce * angle);
        }
    }

#if UNITY_EDITOR
    private void OnDrawGizmosSelected()
    {
        Gizmos.color = Color.yellow;

        if (carRb != null)
            Gizmos.DrawSphere(carRb.transform.position + carRb.centerOfMass, 0.1f);
    }
#endif
}
