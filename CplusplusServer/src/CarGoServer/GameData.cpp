#include "CarGoServer/GameData.hpp"
#include "CarGoServer/Map.hpp"
#include <fmt/core.h>
#include <fmt/color.h>

void GameData::CheckGameStatus(Map& map)
{
	bool playerLeft = false;
	bool allInfected = true;
	bool allSurvivor = true;

	for (const Player& player : players)
	{
		if (player.peer != nullptr && !player.IsPending())
		{
			playerLeft = true;
			if (player.isInfected)
			{
				allSurvivor = false;

				if (!allInfected)
					break;
			}
			else
			{
				allInfected = false;

				if (!allSurvivor)
					break;
			}
		}
	}

	if (!playerLeft)
	{
		fmt::print(stderr, fg(fmt::color::red), "No player left in the lobby. Closing ...\n");

		// No player left on this server
		// Reinitialize
		map.Clear(*this);

		for (Player& player : players)
		{
			player.isInfected = false;
			player.ready = false;
		}

		waitingStateInit = false;
		state = GameState::LOBBY;
	}
	else if (allInfected)
	{
		// End of the game
		lastGameInfectedWins = true;
		state = GameState::GAME_FINISHED;
	}
	else if(allSurvivor)
	{
		lastGameInfectedWins = false;
		state = GameState::GAME_FINISHED;
	}
}