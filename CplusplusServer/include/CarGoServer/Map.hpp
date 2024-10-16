#pragma once

#include <physx/PxPhysicsAPI.h>
#include <physx/cooking/PxCooking.h>
#include "CarGoServer/MapData.hpp"

class GameData;

class Map
{
public:
	Map();
	~Map();

	Map(const Map&) = delete;
	Map(Map&&) = delete;

	Map& operator=(const Map&) = delete;
	Map& operator=(Map&&) = delete;

	void UpdatePhysics(float elapsedTime);
	void Clear();
	physx::PxRigidDynamic* CreateRigidCar(std::uint8_t spawnSlotId, bool isInfected);
	void InitPlayers(GameData& gameData);
	void InitPhysics();
	void UnserializeMap(std::string mapPath);
	void Release();

	physx::PxFoundation* GetFoundation() const;
	physx::PxPhysics* GetPhysics() const;

private:
	MapData m_mapData;
	physx::PxDefaultAllocator m_gAllocator;
	physx::PxDefaultErrorCallback m_gErrorCallback;

	physx::PxFoundation* m_gFoundation = nullptr;
	physx::PxPhysics* m_gPhysics = nullptr;
	physx::PxDefaultCpuDispatcher* m_gDispatcher = nullptr;
	physx::PxScene* m_gScene = nullptr;
	physx::PxMaterial* m_gMaterial = nullptr;
	physx::PxPvd* m_gPvd = nullptr;
};