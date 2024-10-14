#pragma once
#include <cstdint>

const std::uint16_t AppPort = 14769;

const int MinPlayerCount = 2;
const int MaxPlayerCount = 20;

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