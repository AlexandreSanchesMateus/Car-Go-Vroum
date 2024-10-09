#pragma once

#include "CarGoServer/Constant.hpp"
#include <vector>

struct GameData
{
	std::vector<Player> players;
	GameState state;
};