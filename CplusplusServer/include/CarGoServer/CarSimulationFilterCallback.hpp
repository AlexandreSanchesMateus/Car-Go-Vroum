#pragma once

#include <physx/PxPhysicsAPI.h>

class CarSimulationFilterCallback : public physx::PxSimulationFilterCallback
{
public:
    virtual	physx::PxFilterFlags pairFound(physx::PxU64 pairID,
        physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, const physx::PxActor* a0, const physx::PxShape* s0,
        physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, const physx::PxActor* a1, const physx::PxShape* s1,
        physx::PxPairFlags& pairFlags) override;

    virtual	void pairLost(physx::PxU64 pairID,
        physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
        physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
        bool objectRemoved) override;

    virtual	bool statusChange(physx::PxU64& pairID, physx::PxPairFlags& pairFlags, physx::PxFilterFlags& filterFlags) override;
};
