#include "CarGoServer/CarSimulationEventCallback.hpp"
#include "CarGoServer/ClientCar.hpp"
#include "CarGoServer/Protocol.hpp"

#include <fmt/core.h>

CarSimulationEventCallback::CarSimulationEventCallback(GameData& gameData, Map& map) : m_gameData(gameData), m_map(map)
{
}

void CarSimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
{
}

void CarSimulationEventCallback::onWake(physx::PxActor** actors, physx::PxU32 count)
{
}

void CarSimulationEventCallback::onSleep(physx::PxActor** actors, physx::PxU32 count)
{
}

void CarSimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    physx::PxRigidDynamic* actor0 = pairHeader.actors[0]->is<physx::PxRigidDynamic>();
    physx::PxRigidDynamic* actor1 = pairHeader.actors[1]->is<physx::PxRigidDynamic>();

    if (actor0 == nullptr || actor1 == nullptr)
        return;

    ClientCar* car0 = reinterpret_cast<ClientCar*>(actor0->userData);
    ClientCar* car1 = reinterpret_cast<ClientCar*>(actor1->userData);

    if (car0 && car1)
    {
        fmt::println("Collision Detected");

        auto itCar0 = std::find_if(m_gameData.players.begin(), m_gameData.players.end(), [&](const Player& player) { return player.car.get() == car0; });
        auto itCar1 = std::find_if(m_gameData.players.begin(), m_gameData.players.end(), [&](const Player& player) { return player.car.get() == car1; });

        if (itCar0 != m_gameData.players.end() && itCar1 != m_gameData.players.end())
        {
            if (itCar0->isInfected ^ itCar1->isInfected)
            {
                PlayerInfectedPacket packet;

                if (itCar0->isInfected)
                {
                    packet.playerIndex = itCar1->index;
                    itCar1->isInfected = true;
                }
                else
                {
                    packet.playerIndex = itCar0->index;
                    itCar0->isInfected = true;
                }

                ENetPacket* playerInfectedPacket = build_packet<PlayerInfectedPacket>(packet, ENET_PACKET_FLAG_RELIABLE);
                for (const Player& player : m_gameData.players)
                {
                    if (player.peer != nullptr && !player.IsPending())
                        enet_peer_send(player.peer, 0, playerInfectedPacket);
                }

                m_gameData.CheckGameStatus(m_map);
            }
        }
    }
}

void CarSimulationEventCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
}

void CarSimulationEventCallback::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
{
}