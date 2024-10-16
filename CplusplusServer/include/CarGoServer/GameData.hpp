#pragma once

#include "CarGoServer/Constant.hpp"
#include "CarGoServer/PlayerData.hpp"
#include "CarGoServer/Map.hpp"
#include <vector>

struct GameData
{
	std::vector<Player> players;
	GameState state;
	Map map;

	bool waitingStateInit = false;

	std::uint32_t startTimer;
	std::uint32_t timer;
};