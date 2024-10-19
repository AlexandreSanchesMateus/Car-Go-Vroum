#include "CarGoServer/GameData.hpp"
#include <fmt/core.h>
#include <fmt/color.h>

void GameData::CheckGameStatus()
{
	bool playerLeft = false;
	bool allInfected = true;

	for (const Player& player : players)
	{
		if (player.peer != nullptr && !player.IsPending())
		{
			playerLeft = true;
			if (!player.isInfected)
			{
				allInfected = false;
				break;
			}
		}
	}

	if (!playerLeft)
	{
		fmt::print(stderr, fg(fmt::color::red), "No player left in the lobby. Closing ...\n");

		// No player left on this server
		// Reinitialize
		waitingStateInit = false;
		map.Clear(*this);
		state = GameState::LOBBY;
	}
	else if (allInfected)
	{
		// End of the game
		lastGameInfectedWins = true;
		state = GameState::GAME_FINISHED;
	}
}