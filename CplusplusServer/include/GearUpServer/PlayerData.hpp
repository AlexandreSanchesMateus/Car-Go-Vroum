#pragma once

#include "GearUpServer/ClientCar.hpp"
#include "GearUpServer/PlayerInput.hpp"
#include "GearUpServer/Protocol.hpp"
#include <enet6/enet.h>
#include <cstdint>
#include <string>
#include <queue>

struct Player
{
	ENetPeer* peer = nullptr;
	std::uint16_t index;
	std::string name;

	bool ready = false;
	bool isInfected = false;
	std::uint8_t spawnSlotId;

	std::shared_ptr<ClientCar> car = nullptr;

	std::queue<PlayerInputPacket> inputBuffer;
	PlayerInput lastInput;
	std::uint16_t lastInputIndex = 1;

	bool IsPending() const;
};