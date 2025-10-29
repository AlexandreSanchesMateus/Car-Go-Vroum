#pragma once
#include <physx/PxPhysicsAPI.h>
#include "GearUpServer/GameData.hpp"

class CarSimulationEventCallback : public physx::PxSimulationEventCallback
{
public:
    CarSimulationEventCallback(GameData& gameData, Map& map);

    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override;
    void onWake(physx::PxActor** actors, physx::PxU32 count) override;
    void onSleep(physx::PxActor** actors, physx::PxU32 count) override;
    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;
    void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override;

private:
    GameData& m_gameData;
    Map& m_map;
};
