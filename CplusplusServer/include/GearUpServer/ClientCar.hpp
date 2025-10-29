#pragma once

#include "GearUpServer/Timeline.hpp"
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

	float GetCurrentTurnAngle() const;
	std::uint16_t GetCarIndex() const;

private:
	bool UpdateWheelPhysics(WheelData& wheelData, const Timeline& frictionTimeLine, const PlayerInput& inputs, float deltaTime);

	std::uint16_t m_playerIndex;

	WheelData m_frontLeftWheel;
	WheelData m_frontRightWheel;
	WheelData m_rearLeftWheel;
	WheelData m_rearRightWheel;

	Timeline m_frontTireFriction;
	Timeline m_rearTireFriction;
	Timeline m_virtualEngine;

	bool m_fligging;
	float m_flipTimer;

	float m_currentTurnAngle;
	float m_frontRearDistance;
	float m_rearWheelDistance;

	physx::PxRigidDynamic* m_actor;
	physx::PxScene* m_gScene;
	physx::PxPhysics* m_gPhysics;
};