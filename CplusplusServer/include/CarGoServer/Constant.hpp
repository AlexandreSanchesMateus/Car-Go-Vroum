#pragma once
#include <cstdint>

const std::uint16_t AppPort = 14769;

const int MinPlayerCount = 4;
const int MaxPlayerCount = 20;

enum class GameState
{
	WAITING,         // attendre que tout les joueurs soient prets
	RUNNING,         // la partie est en cours
	FINISHED         // une des deux équipes a gagné
};