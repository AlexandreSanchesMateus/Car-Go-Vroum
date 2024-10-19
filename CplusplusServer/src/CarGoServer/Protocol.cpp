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
	Serialize_u8(byteArray, ready ? 1 : 0);
}

PlayerReadyPacket PlayerReadyPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerReadyPacket packet;
	packet.ready = Deserialize_u8(byteArray, offset) == 1;

	return packet;
}

void PlayerInputPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u16(byteArray, inputIndex);
	inputs.Serialize(byteArray);
}

PlayerInputPacket PlayerInputPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerInputPacket packet;
	packet.inputIndex = Deserialize_u16(byteArray, offset);
	packet.inputs = PlayerInput::Deserialize(byteArray, offset);

	return packet;
}

void PlayerConnectPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u16(byteArray, playerIndex);
	Serialize_str(byteArray, name);
	Serialize_u8(byteArray, ready ? 1 : 0);
}

PlayerConnectPacket PlayerConnectPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerConnectPacket packet;
	packet.playerIndex = Deserialize_u16(byteArray, offset);
	packet.name = Deserialize_str(byteArray, offset);
	packet.ready = Deserialize_u8(byteArray, offset) == 1;

	return packet;
}

void PlayerDisconnectedPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u16(byteArray, playerIndex);
}

PlayerDisconnectedPacket PlayerDisconnectedPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerDisconnectedPacket packet;
	packet.playerIndex = Deserialize_u16(byteArray, offset);

	return packet;
}

void ReadyPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u16(byteArray, playerIndex);
	Serialize_u8(byteArray, ready ? 1 : 0);
}

ReadyPacket ReadyPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	ReadyPacket packet;
	packet.playerIndex = Deserialize_u16(byteArray, offset);
	packet.ready = Deserialize_u8(byteArray, offset) == 1;

	return packet;
}

void GameDataPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u16(byteArray, targetPlayerIndex);
	Serialize_u8(byteArray, playerList.size());

	for (const GameDataPacket::PlayerPacketData& data : playerList)
	{
		Serialize_u16(byteArray, data.index);
		Serialize_str(byteArray, data.name);
		Serialize_u8(byteArray, data.ready ? 1 : 0);
	}
}

GameDataPacket GameDataPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	GameDataPacket packet;
	packet.targetPlayerIndex = Deserialize_u16(byteArray, offset);

	std::uint8_t size = Deserialize_u8(byteArray, offset);

	for (int i = 0; i < size; ++i)
	{
		GameDataPacket::PlayerPacketData player;
		player.index = Deserialize_u16(byteArray, offset);
		player.name = Deserialize_str(byteArray, offset);
		player.ready = Deserialize_u8(byteArray, offset) == 1;

		packet.playerList.push_back(player);
	}

	return packet;
}

void GameStateRunningPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u8(byteArray, playerList.size());

	for (const GameStateRunningPacket::RunningPacketData& data : playerList)
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

		packet.playerList.push_back(state);
	}

	return packet;
}

void GameStateStartMovePacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u8(byteArray, moveInfected ? 1 : 0);
}

GameStateStartMovePacket GameStateStartMovePacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	GameStateStartMovePacket packet;
	packet.moveInfected = Deserialize_u8(byteArray, offset) != 0;

	return packet;
}

void GameStateStartPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u32(byteArray, gameDuration);
}

GameStateStartPacket GameStateStartPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	GameStateStartPacket packet;
	packet.gameDuration = Deserialize_u32(byteArray, offset);

	return packet;
}

void GameStateFinishPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u8(byteArray, infectedWins ? 1 : 0);
}

GameStateFinishPacket GameStateFinishPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	GameStateFinishPacket packet;
	packet.infectedWins = Deserialize_u8(byteArray, offset) != 0;

	return packet;
}

void PlayersStatePacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	// Prediction / Reconciliation
	Serialize_u16(byteArray, inputIndex);
	Serialize_f32(byteArray, localTurnAngle);

	// position
	Serialize_f32(byteArray, localPosition.x);
	Serialize_f32(byteArray, localPosition.y);
	Serialize_f32(byteArray, localPosition.z);

	// rotation
	Serialize_f32(byteArray, localRotation.x);
	Serialize_f32(byteArray, localRotation.y);
	Serialize_f32(byteArray, localRotation.z);
	Serialize_f32(byteArray, localRotation.w);

	// velocity
	Serialize_u8(byteArray, localAtRest ? 1 : 0);
	if (!localAtRest)
	{
		// linear
		Serialize_f32(byteArray, localLinearVelocity.x);
		Serialize_f32(byteArray, localLinearVelocity.y);
		Serialize_f32(byteArray, localLinearVelocity.z);

		// angular
		Serialize_f32(byteArray, localAngularVelocity.x);
		Serialize_f32(byteArray, localAngularVelocity.y);
		Serialize_f32(byteArray, localAngularVelocity.z);

		Serialize_f32(byteArray, localFrontLeftWheelVelocity);
		Serialize_f32(byteArray, localFrontRightWheelVelocity);
		Serialize_f32(byteArray, localRearLeftWheelVelocity);
		Serialize_f32(byteArray, localRearRightWheelVelocity);
	}

	Serialize_u8(byteArray, otherPlayersState.size());
	for (const PlayerState& player : otherPlayersState)
	{
		Serialize_u16(byteArray, player.playerIndex);
		player.inputs.Serialize(byteArray);
		Serialize_f32(byteArray, player.turnAngle);

		// position
		Serialize_f32(byteArray, player.position.x);
		Serialize_f32(byteArray, player.position.y);
		Serialize_f32(byteArray, player.position.z);

		// rotation
		Serialize_f32(byteArray, player.rotation.x);
		Serialize_f32(byteArray, player.rotation.y);
		Serialize_f32(byteArray, player.rotation.z);
		Serialize_f32(byteArray, player.rotation.w);

		// velocity
		Serialize_u8(byteArray, player.atRest ? 1 : 0);
		if (!player.atRest)
		{
			// linear
			Serialize_f32(byteArray, player.linearVelocity.x);
			Serialize_f32(byteArray, player.linearVelocity.y);
			Serialize_f32(byteArray, player.linearVelocity.z);

			// angular
			Serialize_f32(byteArray, player.angularVelocity.x);
			Serialize_f32(byteArray, player.angularVelocity.y);
			Serialize_f32(byteArray, player.angularVelocity.z);

			Serialize_f32(byteArray, player.frontLeftWheelVelocity);
			Serialize_f32(byteArray, player.frontRightWheelVelocity);
			Serialize_f32(byteArray, player.rearLeftWheelVelocity);
			Serialize_f32(byteArray, player.rearRightWheelVelocity);
		}
	}
}

PlayersStatePacket PlayersStatePacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayersStatePacket packet;

	packet.inputIndex = Deserialize_u16(byteArray, offset);
	packet.localTurnAngle = Deserialize_f32(byteArray, offset);

	packet.localPosition.x = Deserialize_f32(byteArray, offset);
	packet.localPosition.y = Deserialize_f32(byteArray, offset);
	packet.localPosition.z = Deserialize_f32(byteArray, offset);

	packet.localRotation.x = Deserialize_f32(byteArray, offset);
	packet.localRotation.y = Deserialize_f32(byteArray, offset);
	packet.localRotation.z = Deserialize_f32(byteArray, offset);
	packet.localRotation.w = Deserialize_f32(byteArray, offset);

	packet.localAtRest = Deserialize_u8(byteArray, offset) != 0;
	if (!packet.localAtRest)
	{
		packet.localLinearVelocity.x = Deserialize_f32(byteArray, offset);
		packet.localLinearVelocity.y = Deserialize_f32(byteArray, offset);
		packet.localLinearVelocity.z = Deserialize_f32(byteArray, offset);

		packet.localAngularVelocity.x = Deserialize_f32(byteArray, offset);
		packet.localAngularVelocity.y = Deserialize_f32(byteArray, offset);
		packet.localAngularVelocity.z = Deserialize_f32(byteArray, offset);

		packet.localFrontLeftWheelVelocity = Deserialize_f32(byteArray, offset);
		packet.localFrontRightWheelVelocity = Deserialize_f32(byteArray, offset);
		packet.localRearLeftWheelVelocity = Deserialize_f32(byteArray, offset);
		packet.localRearRightWheelVelocity = Deserialize_f32(byteArray, offset);
	}

	std::uint8_t size = Deserialize_u8(byteArray, offset);
	for (int i = 0; i < size; ++i)
	{
		PlayerState player;
		player.playerIndex = Deserialize_u16(byteArray, offset);
		player.inputs = PlayerInput::Deserialize(byteArray, offset);
		player.turnAngle = Deserialize_f32(byteArray, offset);

		player.position.x = Deserialize_f32(byteArray, offset);
		player.position.y = Deserialize_f32(byteArray, offset);
		player.position.z = Deserialize_f32(byteArray, offset);

		player.rotation.x = Deserialize_f32(byteArray, offset);
		player.rotation.y = Deserialize_f32(byteArray, offset);
		player.rotation.z = Deserialize_f32(byteArray, offset);
		player.rotation.w = Deserialize_f32(byteArray, offset);

		player.atRest = Deserialize_u8(byteArray, offset) != 0;
		if (!player.atRest)
		{
			player.linearVelocity.x = Deserialize_f32(byteArray, offset);
			player.linearVelocity.y = Deserialize_f32(byteArray, offset);
			player.linearVelocity.z = Deserialize_f32(byteArray, offset);

			player.angularVelocity.x = Deserialize_f32(byteArray, offset);
			player.angularVelocity.y = Deserialize_f32(byteArray, offset);
			player.angularVelocity.z = Deserialize_f32(byteArray, offset);

			player.frontLeftWheelVelocity = Deserialize_f32(byteArray, offset);
			player.frontRightWheelVelocity = Deserialize_f32(byteArray, offset);
			player.rearLeftWheelVelocity = Deserialize_f32(byteArray, offset);
			player.rearRightWheelVelocity = Deserialize_f32(byteArray, offset);
		}

		packet.otherPlayersState.push_back(player);
	}

	return packet;
}

void PlayerInfectedPacket::Serialize(std::vector<std::uint8_t>& byteArray) const
{
	Serialize_u16(byteArray, playerIndex);
}

PlayerInfectedPacket PlayerInfectedPacket::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerInfectedPacket packet;
	packet.playerIndex = Deserialize_u16(byteArray, offset);

	return packet;
}
