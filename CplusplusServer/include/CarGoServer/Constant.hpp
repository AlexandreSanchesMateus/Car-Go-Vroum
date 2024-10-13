#pragma once
#include <cstdint>
#include <tuple>

// #include <PxVec3.h>

const std::uint16_t AppPort = 14769;

const float TickRate = 50 / 1; // toute les 0.02 second, ou 50 par seconde (unity default FixedUpdate rate)

const int MinPlayerCount = 2;
const int MaxPlayerCount = 20;

const float InfectedPercentage = 0.25;

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