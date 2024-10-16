#include "CarGoServer/Constant.hpp"
#include "CarGoServer/ClientCar.hpp"

ClientCar::ClientCar(physx::PxRigidDynamic* dynamicCar, physx::PxScene* scene, physx::PxPhysics* physics)
{
	m_actor = dynamicCar;
	m_gScene = scene;
	m_gPhysics = physics;
}

ClientCar::~ClientCar()
{

}

void ClientCar::UpdatePhysics()
{            
	
}

bool ClientCar::UpdateWheelPhysics()
{
	physx::PxTransform wheelPose = m_actor->getGlobalPose(); 

	physx::PxQuat rotation = wheelPose.q;

	physx::PxVec3 localDown(0.0f, -1.0f, 0.0f);  
	physx::PxVec3 down = rotation.rotate(localDown);

	const physx::PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
	physx::PxRaycastHit hitBuffer[bufferSize];  // [out] User provided buffer for results
	physx::PxRaycastBuffer buf(hitBuffer, bufferSize);

	// Raycast against all static & dynamic objects (no filtering)
	// The main result from this call are all hits along the ray, stored in 'hitBuffer'
	m_gScene->raycast(wheelPose.p, down, m_restDistance, buf);

	return buf.nbTouches > 0;
}
