﻿#pragma once

#include "CarGoServer/MapData.hpp"
#include "CarGoServer/ClientCar.hpp"
#include "CarGoServer/CarSimulationEventCallback.hpp"
#include <physx/PxPhysicsAPI.h>
#include <vector>
#include <memory>

struct GameData;

class Map
{
public:
	Map(GameData& gameData);
	Map() = delete;
	~Map();

	Map(const Map&) = delete;
	Map(Map&&) = delete;

	Map& operator=(const Map&) = delete;
	Map& operator=(Map&&) = delete;

	void SimulatePhysics(float elapsedTime);
	void Clear(GameData& gameData);
	physx::PxRigidDynamic* CreateRigidCar(std::uint8_t spawnSlotId, bool isInfected);
	void InitPlayers(GameData& gameData);
	void InitPhysics();
	void UnserializeMap(std::string mapPath);
	void Release();

	physx::PxFoundation* GetFoundation() const;
	physx::PxPhysics* GetPhysics() const;

private:
	MapData m_mapData;

	std::vector<std::shared_ptr<ClientCar>> m_clientCars;
	CarSimulationEventCallback m_carSimulationCallback;

	physx::PxDefaultAllocator m_gAllocator;
	physx::PxDefaultErrorCallback m_gErrorCallback;
	
	physx::PxFoundation* m_gFoundation = nullptr;
	physx::PxPhysics* m_gPhysics = nullptr;
	physx::PxDefaultCpuDispatcher* m_gDispatcher = nullptr;
	physx::PxScene* m_gScene = nullptr;
	physx::PxMaterial* m_gMaterial = nullptr;
	physx::PxPvd* m_gPvd = nullptr;
};