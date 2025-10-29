#pragma once

#include "GearUpServer/PlayerInput.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <enet6/enet.h>
#include <physx/foundation/PxVec3.h>
#include <physx/foundation/PxQuat.h>


enum class Opcode : std::uint8_t
{
	// client related
	// init
	C_PlayerName,          // init player name, send once after client connection

	// runtime
	C_PlayerReady,         // change player ready state, need all player ready to start
	C_PlayerInputs,        // inputs of a player

	// server related
	// init
	S_GameData,            // send the player's id + list players, send once after player's name initialization
	S_PlayerConnected,
	S_PlayerDisconnected,
	S_Ready,

	// runtime
	S_RunningState,        // the game has started (where is the player and in witch team)
	S_StartMovingState,         // start moving after a certain time
	S_StartGameState,
	S_FinishedState,       // the game have finished (who wins)
	S_PlayersState,
	S_PlayerInfected,
};


struct PlayerNamePacket
{
	static constexpr Opcode opcode = Opcode::C_PlayerName;

	std::string name;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerNamePacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct PlayerReadyPacket
{
	static constexpr Opcode opcode = Opcode::C_PlayerReady;

	bool ready;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerReadyPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct PlayerInputPacket
{
	static constexpr Opcode opcode = Opcode::C_PlayerInputs;

	std::uint16_t inputIndex;
	PlayerInput inputs;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerInputPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct PlayerConnectPacket
{
	static constexpr Opcode opcode = Opcode::S_PlayerConnected;

	std::uint16_t playerIndex;
	std::string name;
	bool ready;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerConnectPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct PlayerDisconnectedPacket
{
	static constexpr Opcode opcode = Opcode::S_PlayerDisconnected;

	std::uint16_t playerIndex;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerDisconnectedPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct ReadyPacket
{
	static constexpr Opcode opcode = Opcode::S_Ready;

	std::uint16_t playerIndex;
	bool ready;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static ReadyPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct GameDataPacket
{
	static constexpr Opcode opcode = Opcode::S_GameData;

	struct PlayerPacketData
	{
		std::uint16_t index;
		std::string name;
		bool ready;
	};

	std::uint16_t targetPlayerIndex;
	std::vector<PlayerPacketData> playerList;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static GameDataPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct GameStateRunningPacket
{
	static constexpr Opcode opcode = Opcode::S_RunningState;

	struct RunningPacketData
	{
		std::uint16_t playerIndex;
		std::uint8_t slotId;
		bool isInfected;
	};

	std::vector<RunningPacketData> playerList;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static GameStateRunningPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct GameStateStartMovePacket
{
	static constexpr Opcode opcode = Opcode::S_StartMovingState;

	bool moveInfected;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static GameStateStartMovePacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct GameStateStartPacket
{
	static constexpr Opcode opcode = Opcode::S_StartGameState;

	std::uint32_t gameDuration;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static GameStateStartPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct GameStateFinishPacket
{
	static constexpr Opcode opcode = Opcode::S_FinishedState;

	bool infectedWins;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static GameStateFinishPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct PlayersStatePacket
{
	static constexpr Opcode opcode = Opcode::S_PlayersState;

	struct PlayerState
	{
		std::uint16_t playerIndex;
		PlayerInput inputs;
		float turnAngle;

		physx::PxVec3 position;
		physx::PxQuat rotation;

		bool atRest;
		physx::PxVec3 linearVelocity;
		physx::PxVec3 angularVelocity;

		float frontLeftWheelVelocity;
		float frontRightWheelVelocity;
		float rearLeftWheelVelocity;
		float rearRightWheelVelocity;
	};

	// Prediction / Reconciliation
	std::uint16_t inputIndex;
	float localTurnAngle;

	physx::PxVec3 localPosition;
	physx::PxQuat localRotation;

	bool localAtRest;
	physx::PxVec3 localLinearVelocity;
	physx::PxVec3 localAngularVelocity;

	float localFrontLeftWheelVelocity;
	float localFrontRightWheelVelocity;
	float localRearLeftWheelVelocity;
	float localRearRightWheelVelocity;

	std::vector<PlayerState> otherPlayersState;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayersStatePacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

struct PlayerInfectedPacket
{
	static constexpr Opcode opcode = Opcode::S_PlayerInfected;

	std::uint16_t playerIndex;

	void Serialize(std::vector<std::uint8_t>& byteArray) const;
	static PlayerInfectedPacket Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
};

void Serialize_f32(std::vector<std::uint8_t>& byteArray, float value);
void Serialize_f32(std::vector<std::uint8_t>& byteArray, std::size_t offset, float value);
void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::int8_t value);
void Serialize_i8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int8_t value);
void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::int16_t value);
void Serialize_i16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int16_t value);
void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::int32_t value);
void Serialize_i32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::int32_t value);
void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::uint8_t value);
void Serialize_u8(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint8_t value);
void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::uint16_t value);
void Serialize_u16(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint16_t value);
void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::uint32_t value);
void Serialize_u32(std::vector<std::uint8_t>& byteArray, std::size_t offset, std::uint32_t value);
void Serialize_str(std::vector<std::uint8_t>& byteArray, const std::string& value);
void Serialize_str(std::vector<std::uint8_t>& byteArray, std::size_t offset, const std::string& value);

float Deserialize_f32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::int8_t Deserialize_i8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::int16_t Deserialize_i16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::int32_t Deserialize_i32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::uint8_t Deserialize_u8(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::uint16_t Deserialize_u16(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::uint32_t Deserialize_u32(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);
std::string Deserialize_str(const std::vector<std::uint8_t>& byteArray, std::size_t& offset);

// Petite fonction d'aide pour construire un packet ENet à partir d'une de nos structures de packet, insère automatiquement l'opcode au début des données
template<typename T> ENetPacket* build_packet(const T& packet, enet_uint32 flags)
{
	// On sérialise l'opcode puis le contenu du packet dans un std::vector<std::uint8_t>
	std::vector<std::uint8_t> byteArray;

	Serialize_u8(byteArray, static_cast<std::uint8_t>(T::opcode));
	packet.Serialize(byteArray);

	// On copie le contenu de ce vector dans un packet enet, et on l'envoie au peer
	return enet_packet_create(byteArray.data(), byteArray.size(), flags);
}