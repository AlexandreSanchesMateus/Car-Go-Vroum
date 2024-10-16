#pragma once

#include "CarGoServer/ClientCar.hpp"
#include "CarGoServer/PlayerInput.hpp"
#include <enet6/enet.h>
#include <cstdint>
#include <string>

struct Player
{
	ENetPeer* peer = nullptr;
	std::uint16_t index;
	std::string name;

	bool ready = false;
	bool isInfected = false;
	std::uint8_t spawnSlotId;

	std::shared_ptr<ClientCar> car = nullptr;
	PlayerInput lastInput;

	bool IsPending() const;
};