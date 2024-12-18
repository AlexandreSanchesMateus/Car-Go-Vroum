#include "CarGoServer/ClientCar.hpp"
#include "CarGoServer/Constant.hpp"
#include "CarGoServer/PlayerInput.hpp"
#include "CarGoServer/Math.hpp"
#include <fmt/core.h>

ClientCar::ClientCar(std::uint16_t playerIndex, physx::PxRigidDynamic* dynamicCar, physx::PxScene* scene, physx::PxPhysics* physics)
	: m_playerIndex(playerIndex), m_actor(dynamicCar), m_gScene(scene), m_gPhysics(physics), m_fligging(false), m_flipTimer(0.f), m_currentTurnAngle(0.f)
{
	// Wheels Positions
	m_frontLeftWheel.suspensionVelocity = 0.f;
	m_frontLeftWheel.wheelTrs = physx::PxTransform(-0.9f, -0.13f, 1.85f);
	m_frontRightWheel.suspensionVelocity = 0.f;
	m_frontRightWheel.wheelTrs = physx::PxTransform(0.9f, -0.13f, 1.85f);
	m_rearLeftWheel.suspensionVelocity = 0.f;
	m_rearLeftWheel.wheelTrs = physx::PxTransform(-0.9f, -0.13f, -1.35f);
	m_rearRightWheel.suspensionVelocity = 0.f;
	m_rearRightWheel.wheelTrs = physx::PxTransform(0.9f, -0.13f, -1.35f);

	// wheel distance constant
	m_frontRearDistance = std::abs(m_rearLeftWheel.wheelTrs.p.z - m_frontLeftWheel.wheelTrs.p.z);
	m_rearWheelDistance = std::abs(m_rearLeftWheel.wheelTrs.p.x - m_rearRightWheel.wheelTrs.p.x);

	// Timeline
	m_frontTireFriction.AddKey(0.f, 0.85f);
	m_frontTireFriction.AddKey(0.1f, 0.8f);
	m_frontTireFriction.AddKey(0.4f, 0.25f);
	m_frontTireFriction.AddKey(0.5f, 0.2f);
	m_frontTireFriction.AddKey(0.6f, 0.18f);
	m_frontTireFriction.AddKey(0.9f, 0.07f);
	m_frontTireFriction.AddKey(1.f, 0.05f);

	m_rearTireFriction.AddKey(0.f, 0.8f);
	m_rearTireFriction.AddKey(0.1f, 0.75f);
	m_rearTireFriction.AddKey(0.5f, 0.17f);
	m_rearTireFriction.AddKey(0.6f, 0.12f);
	m_rearTireFriction.AddKey(1.f, 0.08f);

	m_virtualEngine.AddKey(0.f, 0.5f);
	m_virtualEngine.AddKey(0.05f, 0.54f);
	m_virtualEngine.AddKey(0.25f, 0.96f);
	m_virtualEngine.AddKey(0.3f, 1.f);
	m_virtualEngine.AddKey(0.6f, 1.f);
	m_virtualEngine.AddKey(0.65f, 0.97f);
	m_virtualEngine.AddKey(0.95f, 0.33f);
	m_virtualEngine.AddKey(1.f, 0.3f);

	m_actor->userData = this;
}

void ClientCar::UpdatePhysics(const PlayerInput& inputs, float deltaTime)
{
	// TURN WHEEL
	m_currentTurnAngle = MoveTowards(m_currentTurnAngle, inputs.steer * SteeringAngle, (SteeringAngle / SteeringSpeed) * deltaTime);

	if (m_currentTurnAngle != 0.f)
	{
		// Angle of the other wheel
		float r = m_frontRearDistance / std::sin(m_currentTurnAngle * DegToRad);
		float b = std::sqrt(std::pow(r, 2) - std::pow(m_frontRearDistance, 2));
		float angle = std::atan(m_frontRearDistance / (m_rearWheelDistance + b)) * RadToDeg;

		if (m_currentTurnAngle > 0.f)
		{
			m_frontRightWheel.wheelTrs.q = AngleAxis(m_currentTurnAngle, physx::PxVec3(0.f, 1.f, 0.f));
			m_frontLeftWheel.wheelTrs.q = AngleAxis(angle, physx::PxVec3(0.f, 1.f, 0.f));
		}
		else if (m_currentTurnAngle < 0.f)
		{
			m_frontLeftWheel.wheelTrs.q = AngleAxis(m_currentTurnAngle, physx::PxVec3(0.f, 1.f, 0.f));
			m_frontRightWheel.wheelTrs.q = AngleAxis(-angle, physx::PxVec3(0.f, 1.f, 0.f));
		}
	}
	else
	{
		// Return to 0;
		m_frontLeftWheel.wheelTrs.q = physx::PxQuat(1);
		m_frontRightWheel.wheelTrs.q = physx::PxQuat(1);
	}

	bool fullyGrounded = true;

	fullyGrounded &= UpdateWheelPhysics(m_frontLeftWheel, m_frontTireFriction, inputs, deltaTime);
	fullyGrounded &= UpdateWheelPhysics(m_frontRightWheel, m_frontTireFriction, inputs, deltaTime);
	fullyGrounded &= UpdateWheelPhysics(m_rearLeftWheel, m_rearTireFriction, inputs, deltaTime);
	fullyGrounded &= UpdateWheelPhysics(m_rearRightWheel, m_rearTireFriction, inputs, deltaTime);

	physx::PxVec3 linearVelocity = m_actor->getLinearVelocity();
	if (fullyGrounded)
	{
		if (inputs.acceleration == 0)
		{
			m_actor->setCMassLocalPose(physx::PxTransform(0, -0.05f, 0.25f));

			physx::PxVec3 forward = m_actor->getGlobalPose().q.rotate(physx::PxVec3(0.0f, 0.0f, 1.0f));
			float carSpeed = forward.dot(linearVelocity);
			m_actor->addForce(forward * -carSpeed * 4, physx::PxForceMode::eIMPULSE);
		}
		else
			m_actor->setCMassLocalPose(physx::PxTransform(0, -0.05f, -0.04f));
	}
	else if(!m_fligging && inputs.softRecover)
	{
		// TODO FLIP CAR

		physx::PxVec3 up = m_actor->getGlobalPose().q.rotate(physx::PxVec3(0.0f, 1.0f, 0.0f));
		if (up.dot(linearVelocity) < 0.6f && linearVelocity.magnitude() < 0.05f)
		{
			m_actor->addForce(physx::PxVec3(0.f, 1.f, 0.f) * RecoverForce, physx::PxForceMode::eVELOCITY_CHANGE);
			m_fligging = true;
			m_flipTimer = 0.f;
		}
	}

	if (m_fligging)
	{
		m_flipTimer += deltaTime;
		if (m_flipTimer >= TimeBeforeFliping)
		{
			physx::PxTransform carFlip;
			carFlip.p = m_actor->getGlobalPose().p;
			carFlip.q = physx::PxQuat(0.f, 0.f, 0.f, 1.f);

			m_actor->setGlobalPose(carFlip);
			m_fligging = false;
		}
	}

	//fmt::println("speed : {}", (int)(std::abs(forward.dot(m_actor->getLinearVelocity()) * 3.6f)));
}

bool ClientCar::UpdateWheelPhysics(WheelData& wheelData, const Timeline& frictionTimeLine, const PlayerInput& inputs, float deltaTime)
{
	physx::PxVec3 startPos = m_actor->getGlobalPose().transform(wheelData.wheelTrs.p);
	physx::PxVec3 dir = m_actor->getGlobalPose().q.rotate(physx::PxVec3(0.0f, -1.0f, 0.0f));


	physx::PxRaycastBuffer hitBuffer;
	physx::PxQueryFilterData fd = physx::PxQueryFilterData(physx::PxQueryFlag::eSTATIC);
	if (m_gScene->raycast(startPos, dir, RestDistance, hitBuffer, physx::PxHitFlag::eDEFAULT, fd) && hitBuffer.getNbAnyHits() > 0)
	{
		physx::PxVec3 physicForce(0.f, 0.f, 0.f);

		physx::PxVec3 forward = m_actor->getGlobalPose().q.rotate(physx::PxVec3(0.0f, 0.0f, 1.0f));
		float carSpeed = forward.dot(m_actor->getLinearVelocity());

		// ---------------------- Forward Force (acceleration / break) ----------------------
		physx::PxTransform globalWheelTrs = m_actor->getGlobalPose().transform(wheelData.wheelTrs);
		physx::PxVec3 wheelForward = globalWheelTrs.q.rotate(physx::PxVec3(0.0f, 0.0f, 1.0f));

		if (inputs.brake)
		{
			if (carSpeed < -0.1f)
				physicForce = wheelForward * BreakForce;
			else if (carSpeed > 0.1f)
				physicForce = -wheelForward * BreakForce;
			else
				physicForce = forward * -carSpeed * 600;
		}
		else
		{
			if (inputs.acceleration > 0)
			{
				float normelizeSpeed = Clamp((std::abs(carSpeed) / TopSpeed), 0.f, 1.f);

				if (carSpeed < 0.f)
					physicForce = wheelForward * BreakForce;
				else if (normelizeSpeed < 1.f)
				{
					// Move forward
					float availableTorque = m_virtualEngine.Evaluate(normelizeSpeed) * inputs.acceleration * EngineTorque;
					physicForce = wheelForward * availableTorque;
				}
			}
			else if (inputs.acceleration < 0)
			{
				float normelizeSpeed = Clamp((std::abs(carSpeed) / TopReverseSpeed), 0.f, 1.f);

				if (carSpeed > 0.f)
					physicForce = -wheelForward * BreakForce;
				else if (normelizeSpeed < 1.f)
				{
					// Move backward
					float availableTorque = m_virtualEngine.Evaluate(normelizeSpeed) * inputs.acceleration * EngineTorque * 0.8f;
					physicForce = wheelForward * availableTorque;
				}
			}
		}

		// -------------------------- Vertical Force (suspension) --------------------------
		physx::PxVec3 up = m_actor->getGlobalPose().q.rotate(physx::PxVec3(0.0f, 1.0f, 0.0f));

		float offset = RestDistance - hitBuffer.block.distance;
		float currentVelocity = (wheelData.suspensionVelocity - offset) / deltaTime;
		physicForce += up * ((offset * SpringStrenght) - (currentVelocity * Damping));
		wheelData.suspensionVelocity = offset;

		
		// ------------------------ Horizontal Force (side friction) ------------------------
		physx::PxVec3 wheelVelocity = physx::PxRigidBodyExt::getVelocityAtPos(*m_actor, globalWheelTrs.p);
		physx::PxVec3 wheelRight = globalWheelTrs.q.rotate(physx::PxVec3(1.0f, 0.0f, 0.0f));

		float steeringVel = wheelRight.dot(wheelVelocity);
		float friction = std::abs(steeringVel / wheelVelocity.magnitude());
		float desireVelChange = -steeringVel * frictionTimeLine.Evaluate(friction);
		float desireAccel = desireVelChange / deltaTime;

		physicForce += wheelRight * TireMass * desireAccel;

		//physx::PxRigidBodyExt::addForceAtLocalPos(*m_actor, physicForce, wheelData.wheelTrs.p);
		physx::PxRigidBodyExt::addForceAtPos(*m_actor, physicForce, globalWheelTrs.p);

		return true;
	}

	return false;
}

physx::PxRigidDynamic& ClientCar::GetPhysixActor() const
{
	return *m_actor;
}

float ClientCar::GetFrontLeftWheelVelocity() const
{
	return m_frontLeftWheel.suspensionVelocity;
}

float ClientCar::GetFrontRightWheelVelocity() const
{
	return m_frontRightWheel.suspensionVelocity;
}

float ClientCar::GetRearLeftWheelVelocity() const
{
	return m_rearLeftWheel.suspensionVelocity;
}

float ClientCar::GetRearRightWheelVelocity() const
{
	return m_rearRightWheel.suspensionVelocity;
}

float ClientCar::GetCurrentTurnAngle() const
{
	return m_currentTurnAngle;
}

std::uint16_t ClientCar::GetCarIndex() const
{
	return m_playerIndex;
}
