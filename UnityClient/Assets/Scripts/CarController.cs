using UnityEngine;
using System.Collections.Generic;
using UnityEngine.InputSystem;
using NaughtyAttributes;
using TMPro;
using DG.Tweening;
using System.Collections;
using UnityEngine.InputSystem.XR;

public class CarController : MonoBehaviour
{
    [SerializeField, BoxGroup("Init"), Required]
    private Rigidbody carRb;

    [SerializeField, BoxGroup("Mat Config")]
    private MeshRenderer bodyRenderer;
    [SerializeField, BoxGroup("Mat Config")]
    private Material redCarMat;
    [SerializeField, BoxGroup("Mat Config")]
    private Material blueCarMat;

    [SerializeField, BoxGroup("UI")]
    private TextMeshProUGUI nameTxt;

    [SerializeField, BoxGroup("Wheels")]
    private WheelData LF_wheelData;
    [SerializeField, BoxGroup("Wheels")]
    private WheelData RF_wheelData;
    [SerializeField, BoxGroup("Wheels")]
    private WheelData LB_wheelData;
    [SerializeField, BoxGroup("Wheels")]
    private WheelData RB_wheelData;

    [SerializeField, BoxGroup("Wheels")]
    private float wheelHight;

    [SerializeField, BoxGroup("Suspension Settings")]
    private float restDistance;
    [SerializeField, BoxGroup("Suspension Settings")]
    private float springStrenght;
    [SerializeField, BoxGroup("Suspension Settings")]
    private float damping;
    [SerializeField, BoxGroup("Suspension Settings")]
    private LayerMask groundLayer;

    [SerializeField, BoxGroup("Friction Settings")]
    private float tireMass;
    [SerializeField, BoxGroup("Friction Settings"), Range(0, 1)]
    private float frictionStartSkidmark = 0.6f;
    [SerializeField, BoxGroup("Friction Settings")]
    private AnimationCurve frontTireFriction;
    [SerializeField, BoxGroup("Friction Settings")]
    private AnimationCurve rearTireFriction;

    [SerializeField, BoxGroup("Engine Settings")]
    private float engineTorque;
    [SerializeField, BoxGroup("Engine Settings")]
    private AnimationCurve virtualEngine;
    [SerializeField, BoxGroup("Engine Settings")]
    private float topSpeed;
    [SerializeField, BoxGroup("Engine Settings")]
    private float topReverseSpeed;
    [SerializeField, BoxGroup("Engine Settings")]
    private float breakForce;
    [SerializeField, BoxGroup("Engine Settings")]
    private float noInputFrictionForce;

    [SerializeField, BoxGroup("Steering Settings")]
    private float steeringSpeed;
    [SerializeField, BoxGroup("Steering Settings")]
    private float steeringAngle;

    [SerializeField, BoxGroup("Recover Settings")]
    private float recoverForce = 8f;
    [SerializeField, BoxGroup("Recover Settings")]
    private float timeBeforeFliping = 1f;
    [SerializeField, BoxGroup("Recover Settings")]
    private float flipingForce = 200f;

    public float FrontLeftWheelVelocity { get { return LF_wheelData.suspensionVelocity; } set { RB_wheelData.suspensionVelocity = value; } }
    public float FrontRightWheelVelocity { get { return RF_wheelData.suspensionVelocity; } set { RB_wheelData.suspensionVelocity = value; } }
    public float RearLeftWheelVelocity { get { return LB_wheelData.suspensionVelocity; } set { RB_wheelData.suspensionVelocity = value; } }
    public float RearRightWheelVelocity { get { return RB_wheelData.suspensionVelocity; } set { RB_wheelData.suspensionVelocity = value; } }

    // Input variable
    public float AccelerationInput { get; set; } = 0f;
    public bool NeedToBrake { get; set; } = false;

    public int ShowSpeed { get; private set; }

    private float m_desireTurnAngle;
    private float m_currentTurnAngle;

    public bool m_canFlip { get; private set; } = false;
    private float m_frontRearDistance;
    private float m_rearWheelDistance;

    private Skidmarks m_skidmarksController;


    private void Start()
    {
        m_skidmarksController = GameObject.FindGameObjectWithTag("SkidmarkController").GetComponent<Skidmarks>();

        m_frontRearDistance = Mathf.Abs(LB_wheelData.GetWheelTrs().localPosition.z - LF_wheelData.GetWheelTrs().localPosition.z);
        m_rearWheelDistance = Mathf.Abs(LB_wheelData.GetWheelTrs().localPosition.x - RB_wheelData.GetWheelTrs().localPosition.x);
    }

    private void Update()
    {
        ShowSpeed = (int)Vector3.Dot(transform.forward, carRb.velocity);

        // Faire un float desireTurnRadius et le smoother
        // puis faire le calcule de rotation
        m_currentTurnAngle = Mathf.MoveTowards(m_currentTurnAngle, m_desireTurnAngle, (steeringAngle / steeringSpeed) * Time.deltaTime);
        // float turnRadius = frontRearDistance / Mathf.Sin(Mathf.Deg2Rad * currentTurnAngle);

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

        // Update visual
        LF_wheelData.UpdateRotation(carRb);
        LF_wheelData.UpdateSkidmark(m_skidmarksController, frictionStartSkidmark);

        RF_wheelData.UpdateRotation(carRb);
        RF_wheelData.UpdateSkidmark(m_skidmarksController, frictionStartSkidmark);

        LB_wheelData.UpdateRotation(carRb);
        LB_wheelData.UpdateSkidmark(m_skidmarksController, frictionStartSkidmark);

        RB_wheelData.UpdateRotation(carRb);
        RB_wheelData.UpdateSkidmark(m_skidmarksController, frictionStartSkidmark);
    }

    public void UpdatePhysics()
    {
        bool isFullyGrounded = true;

        // Wheel physic
        isFullyGrounded &= UpdateWheelPhysics(LF_wheelData, frontTireFriction, true);
        isFullyGrounded &= UpdateWheelPhysics(RF_wheelData, frontTireFriction, true);
        isFullyGrounded &= UpdateWheelPhysics(LB_wheelData, rearTireFriction, true);
        isFullyGrounded &= UpdateWheelPhysics(RB_wheelData, rearTireFriction, true);

        // No input ground friction
        if (isFullyGrounded)
        {
            if (AccelerationInput == 0f)
            {
                carRb.centerOfMass = new Vector3(0, -0.05f, 0.25f);

                float carSpeed = Vector3.Dot(transform.forward, carRb.velocity);
                carRb.AddForce(transform.forward * -carSpeed * 200 * Time.fixedDeltaTime, ForceMode.Impulse);
            }
            else
                carRb.centerOfMass = new Vector3(0, -0.05f, -0.04f);
        }
        else
        {
            if(Vector3.Dot(transform.up, Vector3.up) < 0.6f && carRb.velocity.magnitude < 0.05f)
                m_canFlip = true;
            else
                m_canFlip = false;
        }
    }

    bool UpdateWheelPhysics(WheelData wheelData, AnimationCurve frictionCurve, bool driveWheel)
    {
        Transform wheelPos = wheelData.GetWheelTrs();

        Ray ray = new Ray(wheelPos.position, transform.rotation * Vector3.down);
        if (Physics.Raycast(ray, out RaycastHit hitInfo, restDistance, groundLayer))
        {
            // Force applied to the wheel
            Vector3 physicForce = Vector3.zero;

            // ---------------------- Forward Force (acceleration / break) ----------------------
            float carSpeed = Vector3.Dot(transform.forward, carRb.velocity);

            if (NeedToBrake)
            {
                if (carSpeed < -0.1f)
                {
                    physicForce = wheelPos.forward * breakForce;
                    //wheelData.StartSkidmark(1, hitInfo.point, hitInfo.normal);
                }
                else if (carSpeed > 0.1f)
                {
                    physicForce = -wheelPos.forward * breakForce;
                    //wheelData.StartSkidmark(1, hitInfo.point, hitInfo.normal);
                }
                else
                {
                    physicForce = transform.forward * -carSpeed * 600;
                    wheelData.StopSkidmark();
                }
            }
            else
            {
                if (AccelerationInput > 0f)
                {
                    float normelizeSpeed = Mathf.Clamp01(Mathf.Abs(carSpeed) / topSpeed);

                    // Break
                    if (carSpeed < 0f)
                    {
                        physicForce = wheelPos.forward * breakForce;
                        //wheelData.StartSkidmark(100, hitInfo.point, hitInfo.normal);
                    }
                    else if (normelizeSpeed < 1f && driveWheel)
                    {
                        // Move forward
                        float availableTorque = virtualEngine.Evaluate(normelizeSpeed) * AccelerationInput * engineTorque;
                        physicForce = wheelPos.forward * availableTorque;
                    }
                }
                else if (AccelerationInput < 0f)
                {
                    float normelizeSpeed = Mathf.Clamp01(Mathf.Abs(carSpeed) / topReverseSpeed);

                    // Break
                    if (carSpeed > 0f)
                    {
                        physicForce = -wheelPos.forward * breakForce;
                        //wheelData.StartSkidmark(100, hitInfo.point, hitInfo.normal);
                    }
                    else if (normelizeSpeed < 1f && driveWheel)
                    {
                        // Move backward
                        float availableTorque = virtualEngine.Evaluate(normelizeSpeed) * AccelerationInput * engineTorque * 0.8f;
                        physicForce = wheelPos.forward * availableTorque;
                    }
                }  
            }

            // -------------------------- Vertical Force (suspension) --------------------------
            float offset = restDistance - hitInfo.distance;
            float currentVelocity = (wheelData.suspensionVelocity - offset) / Time.fixedDeltaTime;
            physicForce += transform.up * ((offset * springStrenght) - (currentVelocity * damping));
            wheelData.suspensionVelocity = offset;

            // ------------------------ Horizontal Force (side friction) ------------------------
            Vector3 wheelVelocity = carRb.GetPointVelocity(wheelPos.position);
            float steeringVel = Vector3.Dot(wheelPos.right, wheelVelocity);
            float friction = Mathf.Abs(steeringVel / wheelVelocity.magnitude);
            float desireVelChange = -steeringVel * frictionCurve.Evaluate(friction);
            float desireAccel = desireVelChange / Time.fixedDeltaTime;

            physicForce += wheelPos.right * tireMass * desireAccel;

            if (Mathf.Abs(steeringVel) >= 0.1f && friction >= frictionStartSkidmark)
                wheelData.StartSkidmark(friction, hitInfo.point, hitInfo.normal);
            else
                wheelData.StopSkidmark();

            // Applie force to rigidbody
            carRb.AddForceAtPosition(physicForce, wheelPos.position);

            // Move Model
            //model.transform.localPosition = new Vector3(0, -hitInfo.distance + wheelHight, 0);
            wheelData.UpdateHight(hitInfo.distance, wheelHight);

            return true;
        }
        else
        {
            // Move Model
            //model.transform.localPosition = new Vector3(0, -restDistance + wheelHight, 0);
            wheelData.UpdateHight(restDistance, wheelHight);
            wheelData.StopSkidmark();

            return false;
        }
    }

    public void SetDesireTurnAngle(float normalValue)
    {
        m_desireTurnAngle = normalValue * steeringAngle;
    }

    public void SetTurnAngle(float angle, float normalDesireAngle)
    {
        m_currentTurnAngle = angle;
    }

    public void SoftRecover()
    {
        if (m_canFlip)
        {
            carRb.AddForce(Vector3.up * recoverForce, ForceMode.VelocityChange);
            StartCoroutine(FlipCar());
        }
    }

    private IEnumerator FlipCar()
    {
        yield return new WaitForSeconds(timeBeforeFliping);

        float timer = 0f;
        float angle = Vector3.SignedAngle(transform.up, Vector3.up, transform.forward);

        bool startLeft = angle < 0f;
        carRb.AddRelativeTorque(0f, 0f, angle / 180 * flipingForce, ForceMode.VelocityChange);

        while (timer < 1f)
        {
            angle = Vector3.SignedAngle(transform.up, Vector3.up, transform.forward);

            if (startLeft && angle > 0.1f)
                break;
            else if (!startLeft && angle < -0.1f)
                break;

            timer += Time.deltaTime;
            yield return null;
        }

        carRb.angularVelocity = new Vector3(0, 0, 0);
    }

    public void InitController(bool infected, string name = "")
    {
        if (nameTxt != null)
            nameTxt.text = name;

        if(infected)
            SetInfectedModel();
    }

    public void SetInfectedModel()
    {
        bodyRenderer.material = redCarMat;

        if(nameTxt != null)
            nameTxt.color = Color.red;
    }

    public void SetSurvivorModel()
    {
        bodyRenderer.material = blueCarMat;

        if (nameTxt != null)
            nameTxt.color = Color.blue;
    }
}
