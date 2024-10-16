#pragma once
#include <cstdint>
#include <tuple>
#include <string>
#include <physx/foundation/PxVec3.h>
#include <physx/foundation/PxQuat.h>
#include <physx/foundation/PxTransform.h>

const std::uint16_t AppPort = 14769;

const float TickRate = 1 / 50 ; // toute les 0.02 second, ou 50 par seconde (unity default FixedUpdate rate)

const int MinPlayerCount = 2;
const int MaxPlayerCount = 20;

const float InfectedPercentage = 0.25;

// en secondes
const float WaitAfterSurvivorMove = 3.f;
const float WaitAfterInfectedMove = 4.f;
const float GameDuration = 150.f;

const std::string MapPath = "assets/map.json";

const physx::PxTransform SurvivorSpawns[] = {
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
};

const physx::PxTransform InfectedSpawns[] = {
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
	physx::PxTransform(physx::PxVec3(0,0,0), physx::PxQuat(0,0,0,0)),
};

enum class GameState
{
	LOBBY,
	WAITING_GAME_START,
	GAME_STARTED,
	GAME_FINISHED
};

enum class DisconnectReport : std::uint32_t
{
	DISCONNECTED,
	SERVER_END,
	KICK,
	LOBBY_FULL,
	GAME_LAUNCHED
};