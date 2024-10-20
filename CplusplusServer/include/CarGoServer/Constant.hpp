#pragma once

#include <cstdint>
#include <tuple>
#include <string>
#include <physx/foundation/PxVec3.h>
#include <physx/foundation/PxQuat.h>
#include <physx/foundation/PxTransform.h>

constexpr std::uint16_t AppPort = 14769;

constexpr std::uint32_t TickRate = 50; // toute les 0.02 second, ou 50 par seconde (unity default FixedUpdate rate)
constexpr std::uint32_t TickDelay = 1000 / TickRate;

constexpr int MinPlayerCount = 1;
constexpr int MaxPlayerCount = 20;

constexpr float InfectedPercentage = 0.25;

constexpr int InputBufferTargetSize = 4;

// en secondes
constexpr float WaitAfterSurvivorMove = 3.f;
constexpr float WaitAfterInfectedMove = 4.f;
constexpr float GameDuration = 150.f;

const std::string MapPath = "assets/Plane.json";

const physx::PxTransform SurvivorSpawns[] = {
	physx::PxTransform(physx::PxVec3(-40.f, 2.f, -90.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-30.f, 2.f, -90.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-20.f, 2.f, -90.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-10.f, 2.f, -90.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(0.f, 2.f, -90.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-40.f, 2.f, -105.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-30.f, 2.f, -105.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-20.f, 2.f, -105.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-10.f, 2.f, -105.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(0.f, 2.f, -105.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-40.f, 2.f, -120.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-30.f, 2.f, -120.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-20.f, 2.f, -120.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-40.f, 2.f, -135.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f)),
	physx::PxTransform(physx::PxVec3(-30.f, 2.f, -135.f), physx::PxQuat(0.f, 0.f, 0.f, 1.f))
};

const physx::PxTransform InfectedSpawns[] = {
	physx::PxTransform(physx::PxVec3(40.f ,2.f , 90.f), physx::PxQuat(0.f, 1.f, 0.f, 0.f)),
	physx::PxTransform(physx::PxVec3(30.f ,2.f , 90.f), physx::PxQuat(0.f, 1.f, 0.f, 0.f)),
	physx::PxTransform(physx::PxVec3(20.f ,2.f , 90.f), physx::PxQuat(0.f, 1.f, 0.f, 0.f)),
	physx::PxTransform(physx::PxVec3(10.f ,2.f , 90.f), physx::PxQuat(0.f, 1.f, 0.f, 0.f)),
	physx::PxTransform(physx::PxVec3(0.f ,2.f , 90.f), physx::PxQuat(0.f, 1.f, 0.f, 0.f))
};

// Car params
constexpr float RestDistance = 0.8f;
constexpr float SpringStrenght = 6000.f;
constexpr float Damping = 240.f;

constexpr float TireMass = 28.f;

constexpr float EngineTorque = 1800.f;
constexpr float TopSpeed = 40.f;
constexpr float TopReverseSpeed = 18.f;
constexpr float BreakForce = 2000.f;
constexpr float NoInputFrictionForce = 0.9f;

constexpr float SteeringSpeed = 0.45f;
constexpr float SteeringAngle = 25.f;

constexpr float RecoverForce = 7.f;
constexpr float TimeBeforeFliping = 0.6f;
constexpr float FlipingForce = 7.f;


enum class GameState
{
	LOBBY,
	WAITING_GAME_START,
	GAME_STARTED,
	GAME_FINISHED
};

enum class DisconnectReport : std::uint32_t
{
	DISCONNECTED,
	SERVER_END,
	KICK,
	LOBBY_FULL,
	GAME_LAUNCHED
};