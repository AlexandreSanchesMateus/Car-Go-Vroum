#include "CarGoServer/CarSimulationEventCallback.hpp"
#include "CarGoServer/Protocol.hpp"

CarSimulationEventCallback::CarSimulationEventCallback(GameData& gameData)
    : m_gameData(gameData)
{}

void CarSimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    physx::PxRigidActor* actor0 = static_cast<physx::PxRigidActor*>(pairHeader.actors[0]);
    physx::PxRigidActor* actor1 = static_cast<physx::PxRigidActor*>(pairHeader.actors[1]);

    ClientCar* car0 = reinterpret_cast<ClientCar*>(actor0->userData);
    ClientCar* car1 = reinterpret_cast<ClientCar*>(actor1->userData);

    if (car0 && car1)
    {
        auto itCar0 = std::find_if(m_gameData.players.begin(), m_gameData.players.end(), [&](const Player& player) { return player.car == car0; });
        auto itCar1 = std::find_if(m_gameData.players.begin(), m_gameData.players.end(), [&](const Player& player) { return player.car == car1; });

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

                m_gameData.CheckGameStatus();
            }
        }
    }
}
