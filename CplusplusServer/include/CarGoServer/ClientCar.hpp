#pragma once
#include <memory>
#include <PxPhysicsAPI.h>

class ClientCar
{
public:
	ClientCar(physx::PxRigidDynamic* dynamicCar, physx::PxScene* scene, physx::PxPhysics* physics);
	~ClientCar();

	ClientCar(const ClientCar&) = default;
	ClientCar(ClientCar&&) = delete;
	void UpdatePhysics();
	bool UpdateWheelPhysics();

	ClientCar& operator=(const ClientCar&) = delete;
	ClientCar& operator=(ClientCar&&) = delete;

private:
	float m_restDistance;

	physx::PxRigidDynamic* m_actor;
	physx::PxScene* m_gScene;
	physx::PxPhysics* m_gPhysics;
};