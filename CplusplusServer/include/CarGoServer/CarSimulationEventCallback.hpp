#pragma once
#include <physx/PxPhysicsAPI.h>
#include "CarGoServer/ClientCar.hpp"
#include "CarGoServer/GameData.hpp"

class CarSimulationEventCallback : physx::PxSimulationEventCallback
{
public:
    CarSimulationEventCallback(GameData& gameData);

    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;

    //void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override {}
    //void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) override {}
    //void onWake(physx::PxActor**, physx::PxU32) override {}
    //void onSleep(physx::PxActor**, physx::PxU32) override {}

private:
    GameData& m_gameData;
};

