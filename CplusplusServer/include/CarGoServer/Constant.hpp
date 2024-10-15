#pragma once
#include <cstdint>
#include <tuple>
#include <string>
#include <physx/foundation/PxVec3.h>
#include <physx/foundation/PxQuat.h>

const std::uint16_t AppPort = 14769;

const float TickRate = 1 / 50 ; // toute les 0.02 second, ou 50 par seconde (unity default FixedUpdate rate)

const int MinPlayerCount = 2;
const int MaxPlayerCount = 20;

const float InfectedPercentage = 0.25;

const std::string MapPath = "assets/map.json";

// To change for PxVec3
const std::tuple<float, float> SurvivorSpawn[] =
{
	{0.f, 0.f},
	{0.f, 0.f},
	{0.f, 0.f},
	{0.f, 0.f},
	{0.f, 0.f}
};

// To change for PxVec3
const const std::tuple<float, float> InfectedSpawn[] =
{
	{0.f, 0.f},
	{0.f, 0.f},
	{0.f, 0.f},
	{0.f, 0.f},
	{0.f, 0.f}
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