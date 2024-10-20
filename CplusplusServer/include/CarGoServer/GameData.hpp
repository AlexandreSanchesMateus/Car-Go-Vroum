#pragma once

#include "CarGoServer/Constant.hpp"
#include "CarGoServer/PlayerData.hpp"
#include <vector>

class Map;

struct GameData
{
	std::vector<Player> players;
	GameState state;

	bool waitingStateInit = false;
	bool lastGameInfectedWins = false;

	std::uint32_t endTimer;
	std::uint32_t timer;

	void CheckGameStatus(Map& map);
};