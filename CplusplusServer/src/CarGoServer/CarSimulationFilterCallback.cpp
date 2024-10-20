#include "CarGoServer/CarSimulationFilterCallback.hpp"

physx::PxFilterFlags CarSimulationFilterCallback::pairFound(physx::PxU64 pairID, physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, const physx::PxActor* a0, const physx::PxShape* s0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, const physx::PxActor* a1, const physx::PxShape* s1, physx::PxPairFlags& pairFlags)
{
	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT | physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
	return physx::PxFilterFlag::eDEFAULT;
}

void CarSimulationFilterCallback::pairLost(physx::PxU64 pairID, physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, bool objectRemoved)
{
}

bool CarSimulationFilterCallback::statusChange(physx::PxU64& pairID, physx::PxPairFlags& pairFlags, physx::PxFilterFlags& filterFlags)
{
	return false;
}
