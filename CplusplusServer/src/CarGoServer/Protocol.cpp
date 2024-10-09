#include "CarGoServer/Protocol.hpp"
#include <cassert>
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>

// C'est pas beau, mais il est actuellement 2h du matin
inline float ntohf(uint32_t net32)
{
	union {
		float f;
		uint32_t u;
	} value;

	value.u = ntohl(net32);

	return value.f;
}

inline uint32_t htonf(float f32)
{
	union {
		float f;
		uint32_t u;
	} value;

	value.f = f32;

	return ntohl(value.u);
}
#endif


void Serialize_f32(std::vector<std::uint8_t>& byteArray, float value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_f32(byteArray, offset, value);
}

void Serialize_f32(std::vector<std::uint8_t>& byteArray, std::size_t offset, float value)
{
	std::uint32_t v = htonf(value);

	assert(offset + sizeof(v) <= byteArray.size());
	std::memcpy(&byteArray[offset], &v, sizeof(v));
}

void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::int8_t value)
{
	return Serialize_u8(byteArray, static_cast<std::uint8_t>(value));
}

void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int8_t value)
{
	return Serialize_u8(byteArray, offset, static_cast<std::uint8_t>(value));
}

void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::int16_t value)
{
	return Serialize_u16(byteArray, static_cast<std::uint16_t>(value));
}

void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int16_t value)
{
	return Serialize_u16(byteArray, offset, static_cast<std::uint16_t>(value));
}

void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::int32_t value)
{
	return Serialize_u32(byteArray, static_cast<std::uint32_t>(value));
}

void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int32_t value)
{
	return Serialize_u32(byteArray, offset, static_cast<std::uint32_t>(value));
}

void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::uint8_t value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_u8(byteArray, offset, value);
}

void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint8_t value)
{
	assert(offset < byteArray.size());
	byteArray[offset] = value;
}

void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::uint16_t value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_u16(byteArray, offset, value);
}

void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint16_t value)
{
	value = htons(value);

	assert(offset + sizeof(value) <= byteArray.size());
	std::memcpy(&byteArray[offset], &value, sizeof(value));
}

void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::uint32_t value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(value));

	return Serialize_u32(byteArray, offset, value);
}

void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint32_t value)
{
	value = htonl(value);

	assert(offset + sizeof(value) <= byteArray.size());
	std::memcpy(&byteArray[offset], &value, sizeof(value));
}

void Serialize_str(std::vector<std::uint8_t>& byteArray, const std::string& value)
{
	std::size_t offset = byteArray.size();
	byteArray.resize(offset + sizeof(std::uint32_t) + value.size());
	return Serialize_str(byteArray, offset, value);
}

void Serialize_str(std::vector<std::uint8_t>& byteArray, std::size_t offset, const std::string& value)
{
	Serialize_u32(byteArray, offset, static_cast<std::uint32_t>(value.size()));
	offset += sizeof(std::uint32_t);

	if (!value.empty())
		std::memcpy(&byteArray[offset], value.data(), value.size());
}

float Deserialize_f32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint32_t value;
	std::memcpy(&value, &byteArray[offset], sizeof(value));

	float v = ntohf(value);

	offset += sizeof(value);

	return v;
}

std::int8_t Deserialize_i8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return static_cast<std::int8_t>(Deserialize_u8(byteArray, offset));
}

std::int16_t Deserialize_i16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return static_cast<std::int16_t>(Deserialize_u16(byteArray, offset));
}

std::int32_t Deserialize_i32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	return static_cast<std::int32_t>(Deserialize_u32(byteArray, offset));
}

std::uint8_t Deserialize_u8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint8_t value = byteArray[offset];
	offset += sizeof(value);

	return value;
}

std::uint16_t Deserialize_u16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint16_t value;
	std::memcpy(&value, &byteArray[offset], sizeof(value));
	value = ntohs(value);

	offset += sizeof(value);

	return value;
}

std::uint32_t Deserialize_u32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint32_t value;
	std::memcpy(&value, &byteArray[offset], sizeof(value));
	value = ntohl(value);

	offset += sizeof(value);

	return value;
}

std::string Deserialize_str(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	std::uint32_t length = Deserialize_u32(byteArray, offset);
	std::string str(length, ' ');
	std::memcpy(&str[0], &byteArray[offset], length);

	offset += length;

	return str;
}

void PlayerNamePacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_str(byteArray, name);
}

PlayerNamePacket PlayerNamePacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerNamePacket packet;
	packet.name = Deserialize_str(byteArray, offset);

	return packet;
}

void PlayerReadyPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	std::uint8_t byte = ready ? 1 : 0;
	Serialize_u8(byteArray, byte);
}

PlayerReadyPacket PlayerReadyPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerReadyPacket packet;
	packet.ready = Deserialize_u8(byteArray, offset) == 1;

	return packet;
}

void PlayerInputPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_f32(byteArray, inputs.acceleration);
	Serialize_f32(byteArray, inputs.steer);

	std::uint8_t byte = inputs.brake ? 1 : 0;
	Serialize_u8(byteArray, byte);
}

PlayerInputPacket PlayerInputPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerInputPacket packet;
	packet.inputs.acceleration = Deserialize_f32(byteArray, offset);
	packet.inputs.steer = Deserialize_f32(byteArray, offset);
	packet.inputs.brake = Deserialize_u8(byteArray, offset) == 1;

	return packet;
}

void GameDataPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u16(byteArray, playerIndex);
}

GameDataPacket GameDataPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	GameDataPacket packet;
	packet.playerIndex = Deserialize_u16(byteArray, offset);

	return packet;
}

void PlayerListPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u8(byteArray, playerList.size());

	for (const PlayerListPacket::PlayerPacketData& data : playerList)
	{
		Serialize_u16(byteArray, data.index);
		Serialize_str(byteArray, data.name);
		std::uint8_t byte = data.ready ? 1 : 0;
		Serialize_u8(byteArray, byte);
	}
}

PlayerListPacket PlayerListPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerListPacket packet;
	std::uint8_t size = Deserialize_u8(byteArray, offset);

	for (int i = 0; i < size; ++i)
	{
		PlayerListPacket::PlayerPacketData player;
		player.index = Deserialize_u16(byteArray, offset);
		player.name = Deserialize_str(byteArray, offset);
		player.ready = Deserialize_u8(byteArray, offset) == 1;

		packet.playerList.push_back(player);
	}

	return packet;
}

void GameStateRunningPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u8(byteArray, playersState.size());

	for (const GameStateRunningPacket::RunningPacketData& data : playersState)
	{
		Serialize_u16(byteArray, data.playerIndex);
		std::uint8_t byte = ((data.isInfected ? 1 : 0) << 8) | data.slotId;
		Serialize_u8(byteArray, byte);
	}
}

GameStateRunningPacket GameStateRunningPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	GameStateRunningPacket packet;
	std::uint8_t size = Deserialize_u8(byteArray, offset);

	for (int i = 0; i < size; ++i)
	{
		GameStateRunningPacket::RunningPacketData state;
		state.playerIndex = Deserialize_u16(byteArray, offset);
		std::uint8_t byte = Deserialize_u8(byteArray, offset);
		state.slotId = byte & 0b01111111;
		state.isInfected = byte >> 8 == 1;

		packet.playersState.push_back(state);
	}

	return packet;
}
