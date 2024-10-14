#pragma once

#include "CarGoServer/GameData.hpp"
#include <physx/PxPhysicsAPI.h>
#include "MapData.hpp"


class Map
{
public:
	Map();
	~Map();

	Map(const Map&) = delete;
	Map(Map&&) = delete;

	Map& operator=(const Map&) = delete;
	Map& operator=(Map&&) = delete;

	void UpdatePhisics();
	void Clear();
	void InitPlayers(const GameData& gameData);
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