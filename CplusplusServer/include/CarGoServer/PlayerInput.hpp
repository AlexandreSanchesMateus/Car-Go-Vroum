#pragma once

#include <cstdint>
#include <vector>

struct PlayerInput
{
	int acceleration = 0;
	int steer = 0;
	bool brake = false;
	bool softRecover = false;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerInput Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};