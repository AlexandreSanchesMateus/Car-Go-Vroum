#pragma once

#include "CarGoServer/Timeline.hpp"
#include <memory>
#include <PxPhysicsAPI.h>

struct PlayerInput;

struct WheelData
{
	physx::PxTransform wheelTrs;
	float suspensionVelocity;
};

class ClientCar
{
public:
	ClientCar(std::uint16_t playerIndex, physx::PxRigidDynamic* dynamicCar, physx::PxScene* scene, physx::PxPhysics* physics);
	~ClientCar() = default;

	ClientCar(const ClientCar&) = default;
	ClientCar(ClientCar&&) = default;

	ClientCar& operator=(const ClientCar&) = delete;
	ClientCar& operator=(ClientCar&&) = default;

	void UpdatePhysics(const PlayerInput& inputs,float deltaTime);
	physx::PxRigidDynamic& GetPhysixActor() const;

	float GetFrontLeftWheelVelocity() const;
	float GetFrontRightWheelVelocity() const;
	float GetRearLeftWheelVelocity() const;
	float GetRearRightWheelVelocity() const;

private:
	bool UpdateWheelPhysics(WheelData& wheelData, const Timeline& frictionTimeLine, const PlayerInput& inputs, float deltaTime);
	float Clamp(float n, float lower, float upper) const;
	float MoveTowards(float current, float target, float maxDelta) const;
	physx::PxQuat AngleAxis(float degrees, physx::PxVec3 axis) const;

	std::uint16_t m_playerIndex;

	WheelData m_frontLeftWheel;
	WheelData m_frontRightWheel;
	WheelData m_rearLeftWheel;
	WheelData m_rearRightWheel;

	float m_restDistance;
	float m_springStrenght;
	float m_damping;

	float m_tireMass;
	Timeline m_frontTireFriction;
	Timeline m_rearTireFriction;

	float m_engineTorque;
	Timeline m_virtualEngine;
	float m_topSpeed;
	float m_topReverseSpeed;
	float m_breakForce;
	float m_noInputFrictionForce;

	float m_steeringSpeed;
	float m_steeringAngle;

	float m_recoverForce;
	float m_timeBeforeFliping;
	float m_flipingForce;

	bool m_canFlip;
	float m_currentTurnAngle;
	float m_frontRearDistance;
	float m_rearWheelDistance;

	physx::PxRigidDynamic* m_actor;
	physx::PxScene* m_gScene;
	physx::PxPhysics* m_gPhysics;
};