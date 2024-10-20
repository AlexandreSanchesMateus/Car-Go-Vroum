#pragma once
#include <physx/PxPhysicsAPI.h>
#include "CarGoServer/GameData.hpp"

class CarSimulationEventCallback : public physx::PxSimulationEventCallback
{
public:
    CarSimulationEventCallback(GameData& gameData, Map& map);

    virtual void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) override {}
    virtual void onWake(physx::PxActor**, physx::PxU32) override {}
    virtual void onSleep(physx::PxActor**, physx::PxU32) override {}
    virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
    virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override {}
    virtual void onAdvance(const physx::PxRigidBody*const*, const physx::PxTransform*, const physx::PxU32) override {}

private:
    GameData& m_gameData;
    Map& m_map;
};
