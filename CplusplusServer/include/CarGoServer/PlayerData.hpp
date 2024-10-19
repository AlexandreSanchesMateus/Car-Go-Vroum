#pragma once

#include "CarGoServer/ClientCar.hpp"
#include "CarGoServer/PlayerInput.hpp"
#include "CarGoServer/Protocol.hpp"
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
	float inputBufferFactor = 0.f;
	PlayerInput lastInput;
	std::uint16_t lastInputIndex;

	bool IsPending() const;
};