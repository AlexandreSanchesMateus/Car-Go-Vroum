#include "CarGoServer/Constant.hpp"
#include "CarGoServer/Protocol.hpp"
#include "CarGoServer/PlayerData.hpp"
#include "CarGoServer/GameData.hpp"
#include "CarGoServer/Command.hpp"
#include "CarGoServer/Map.hpp"
#include <fmt/core.h>
#include <fmt/color.h>
#include <enet6/enet.h>
#include <vector>
#include <memory>
#include <stdexcept>

#include <CarGoServer/Math.hpp>

void handle_message(Player& player, const std::vector<std::uint8_t>& message, GameData& gameData, Map& map, const Command& cmdPrompt);
void PurgePlayers(const GameData& gameData);
ENetPacket* build_game_data_packet(const GameData& gameData, const Player& targetPlayer);
ENetPacket* build_running_state_packet(const GameData& gameData);
ENetPacket* build_player_state_packet(const GameData& gameData, const Player& targetPlayer);
void tick_physics(GameData& gameData, Map& map, float deltaTime);
void tick_logic(GameData& gameData, Map& map, std::uint32_t now, const Command& cmdPrompt);

int main()
{
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "   _________    ____        __________     _____ __________ _    ____________\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "  / ____/   |  / __ \\      / ____/ __ \\   / ___// ____/ __ \\ |  / / ____/ __ \\\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), " / /   / /| | / /_/ /_____/ / __/ / / /   \\__ \\/ __/ / /_/ / | / / __/ / /_/ /\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "/ /___/ ___ |/ _, _/_____/ /_/ / /_/ /   ___/ / /___/ _, _/| |/ / /___/ _, _/\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "\\____/_/  |_/_/ |_|      \\____/\\____/   /____/_____/_/ |_| |___/_____/_/ |_|\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "                                                             by Alexandre SM & Timothe L\n");


	fmt::println("\nInitialization ...");

	if (enet_initialize() != 0)
	{
		fmt::print(stderr, fg(fmt::color::red), "Failed to initialize ENet\n");
		return EXIT_FAILURE;
	}

	fmt::print("    => ");
	fmt::print(stderr, fg(fmt::color::green), "ENet Initialized\n");

	// Création de l'hôte serveur
	ENetAddress address;
	enet_address_build_any(&address, ENET_ADDRESS_TYPE_IPV6);
	address.port = AppPort;

	ENetHost* host = enet_host_create(ENET_ADDRESS_TYPE_ANY, &address, MaxPlayerCount + 2, 0, 0, 0);
	if (!host)
	{
		fmt::print(stderr, fg(fmt::color::red), "Failed to create ENet host\n");
		return EXIT_FAILURE;
	}

	fmt::print("    => ");
	fmt::print(stderr, fg(fmt::color::green), "ENet host created\n\n");

	GameData gameData;
	gameData.state = GameState::LOBBY;
	gameData.lastGameInfectedWins = false;
	gameData.waitingStateInit = false;

	Map map(gameData);

	fmt::println("< ======================================== >");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "     ___ ___   _   _____   __\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "    | _ \\ __| /_\\ |   \\ \\ / /\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "    |   / _| / _ \\| |) \\ V /\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "    |_|_\\___/_/ \\_\\___/ |_|\n");
	fmt::println("Application port : {}\n", AppPort);

	Command cmdPrompt;

	std::uint32_t now = enet_time_get();
	std::uint32_t nextTick = now + TickDelay;
	std::uint32_t lastTick = now;

	bool serverOpen = true;
	while (serverOpen)
	{
		now = enet_time_get();

		ENetEvent event;
		if (enet_host_service(host, &event, 1) > 0)
		{
			do
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
				{
					auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) { return player.peer == nullptr; });
					if (it == gameData.players.end())
					{
						// Pas de slot libre, on en rajoute un
						auto& player = gameData.players.emplace_back();
						player.index = gameData.players.size() - 1;
						it = gameData.players.end() - 1;
					}

					Player& player = *it;
					player.peer = event.peer;
					player.ready = false;
					player.isInfected = false;
					player.name.clear();

					cmdPrompt.ClearLastPrompt();
					fmt::print(stderr, fg(fmt::color::green), "[+]");
					fmt::println(" Player #{} connected", player.index);
					cmdPrompt.RecoverLastPrompt();
				}
					break;

				case ENET_EVENT_TYPE_DISCONNECT:
				case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
				{
					auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) { return player.peer == event.peer; });
					if (it == gameData.players.end())
						throw std::runtime_error("No player found");

					Player& player = *it;
					player.peer = nullptr;

					cmdPrompt.ClearLastPrompt();
					fmt::print(stderr, fg(fmt::color::red), "[-]");

					if (event.type == ENET_EVENT_TYPE_DISCONNECT_TIMEOUT)
						fmt::print(" (time out)");

					if (!player.IsPending())
					{
						fmt::println(" Player #{} disconnected (aka {})", player.index, player.name);

						// envoyer aux joueur que une joueur c'est déconnecté
						PlayerDisconnectedPacket disconnectPacket;
						disconnectPacket.playerIndex = player.index;

						ENetPacket* playerDisconnectPacket = build_packet<PlayerDisconnectedPacket>(disconnectPacket, ENET_PACKET_FLAG_RELIABLE);
						for (const Player& other : gameData.players)
						{
							if(other.peer != nullptr && !other.IsPending())
								enet_peer_send(other.peer, 0, playerDisconnectPacket);
						}

						// Check game status
						if (gameData.state != GameState::LOBBY)
							gameData.CheckGameStatus(map);

					}
					else
						fmt::println(" Player #{} disconnected (no name)", player.index);

					// Vérifier s'il y a encore quelqu'un
					// si oui vérifier l'état de la partie
					// si non fermé la partie immédiatement

					cmdPrompt.RecoverLastPrompt();
				}
					break;

				case ENET_EVENT_TYPE_RECEIVE:
				{
					auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&](const Player& player) { return player.peer == event.peer; });
					if (it == gameData.players.end())
						throw std::runtime_error("No player found");
					
					Player& player = *it;

					// On gère le message qu'on a reçu
					std::vector<std::uint8_t> content(event.packet->dataLength);
					std::memcpy(content.data(), event.packet->data, event.packet->dataLength);

					handle_message(player, content, gameData, map, cmdPrompt);
					enet_packet_destroy(event.packet);
				}
					break;

				case ENET_EVENT_TYPE_NONE:
					cmdPrompt.ClearLastPrompt();
					fmt::print(stderr, fg(fmt::color::yellow), "Enet event type : unexpected event\n");
					cmdPrompt.RecoverLastPrompt();
					break;
				}
			} while (enet_host_check_events(host, &event) > 0);
		}

		std::optional<Command::Report> commandReport = cmdPrompt.HandleEvent();
		if (commandReport)
		{
			switch (commandReport->action)
			{
			case Command::Action::Purge:
				cmdPrompt.ClearLastPrompt();
				PurgePlayers(gameData);
				cmdPrompt.RecoverLastPrompt();
				break;

			case Command::Action::Kick:
			{
				auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&commandReport](const Player& other) {return commandReport->params == other.index; });
				if (it != gameData.players.end() && it->peer != nullptr)
					enet_peer_disconnect(it->peer, (std::uint32_t)DisconnectReport::KICK);
				else
				{
					fmt::print(stderr, fg(fmt::color::yellow), "\r[SERV REPLY]");
					fmt::println(" No player found with #{} as id", commandReport->params);
					cmdPrompt.RecoverLastPrompt();
				}
				break;
			}

			case Command::Action::Close:
				serverOpen = false;
				continue;
			}
		}

		// Tick logique
		if (now >= nextTick) 
		{
			float deltaTime = (now - lastTick) / 1000.0f;

			tick_logic(gameData, map, now, cmdPrompt);
			tick_physics(gameData, map, deltaTime);

			lastTick = now;
			nextTick += TickDelay;
		}
	}

	for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
	{
		if (it->peer != nullptr)
			enet_peer_disconnect_now(it->peer, (std::uint32_t)DisconnectReport::SERVER_END);
	}

	enet_deinitialize();
	fmt::print(stderr, fg(fmt::color::green), "\r \nENet Deinitialized\n");

	fmt::print(stderr, fg(fmt::color::medium_spring_green), " ___            _\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "| _ )_  _ ___  | |__ _  _ ___\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "| _ \\ || / -_) | '_ \\ || / -_)\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "|___/\\_, \\___| |_.__/\\_, \\___|\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "     |__/            |__/\n");

	return EXIT_SUCCESS;
}

void handle_message(Player& player, const std::vector<std::uint8_t>& message, GameData& gameData, Map& map, const Command& cmdPrompt)
{
	std::size_t offset = 0;
	Opcode opcode = static_cast<Opcode>(Deserialize_u8(message, offset));
	switch (opcode)
	{
	case Opcode::C_PlayerName:
	{
		PlayerNamePacket playerName = PlayerNamePacket::Deserialize(message, offset);
		player.name = playerName.name;

		int playerCount = 0;
		for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
		{
			if (it->peer != nullptr && !it->IsPending())
				++playerCount;
		}

		if (gameData.state != GameState::LOBBY || playerCount > MaxPlayerCount)
		{
			// Deconnection player
			enet_peer_disconnect(player.peer, gameData.state != GameState::LOBBY ? (std::uint32_t)DisconnectReport::GAME_LAUNCHED : (std::uint32_t)DisconnectReport::LOBBY_FULL);
		}
		else
		{
			// Init player
			ENetPacket* gameDataPacket = build_game_data_packet(gameData, player);
			enet_peer_send(player.peer, 0, gameDataPacket);

			// Broadcast
			PlayerConnectPacket playerConnect;
			playerConnect.playerIndex = player.index;
			playerConnect.name = player.name;
			playerConnect.ready = player.ready;

			ENetPacket* connectPacket = build_packet<PlayerConnectPacket>(playerConnect, ENET_PACKET_FLAG_RELIABLE);
			for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
			{
				if(it->peer != nullptr && !it->IsPending() && it->index != player.index)
					enet_peer_send(it->peer, 0, connectPacket);
			}

			cmdPrompt.ClearLastPrompt();
			fmt::print(stderr, fg(fmt::color::green), "Player #{} (aka {}) successfully initialized\n", player.index, player.name);
			cmdPrompt.RecoverLastPrompt();
		}
	}
		break;

	case Opcode::C_PlayerReady:
	{
		// vérifier si le jeu n'a pas commencer
		if (gameData.state == GameState::LOBBY)
		{
			PlayerReadyPacket playerReady = PlayerReadyPacket::Deserialize(message, offset);
			player.ready = playerReady.ready;

			bool allReady = true;
			int playerCount = 0;
			// vérifier si tout les joueurs son pret
			for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
			{
				if (it->peer == nullptr || it->IsPending())
					continue;

				if (!it->ready)
				{
					allReady = false;
					break;
				}

				++playerCount;
			}

			if (allReady && playerCount >= MinPlayerCount)
			{
				cmdPrompt.ClearLastPrompt();
				PurgePlayers(gameData);
				fmt::println("----------------- GAME STARTED -----------------");

				// init world
				map.InitPlayers(gameData);
				cmdPrompt.RecoverLastPrompt();

				gameData.state = GameState::WAITING_GAME_START;
				gameData.timer = enet_time_get();
				gameData.endTimer = gameData.timer + WaitAfterSurvivorMove * 1000;

				ENetPacket* packet = build_running_state_packet(gameData);
				for (const Player& other : gameData.players)
				{
					if (other.peer != nullptr && !other.IsPending())
						enet_peer_send(other.peer, 0, packet);
				}
			}
			else
			{
				ReadyPacket readyPacket;
				readyPacket.playerIndex = player.index;
				readyPacket.ready = player.ready;

				ENetPacket* packet = build_packet<ReadyPacket>(readyPacket, ENET_PACKET_FLAG_RELIABLE);
				for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
				{
					if (it->peer != nullptr && !it->IsPending())
						enet_peer_send(it->peer, 0, packet);
				}
			}
		}
	}
		break;

	case Opcode::C_PlayerInputs:
	{
		PlayerInputPacket playerInputs = PlayerInputPacket::Deserialize(message, offset);
		player.inputBuffer.push(std::move(playerInputs));
	}
		break;
	}
}

void PurgePlayers(const GameData& gameData)
{
	for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
	{
		if (it->peer != nullptr && it->IsPending())
		{
			enet_peer_disconnect(it->peer, (std::uint32_t)DisconnectReport::KICK);
			fmt::print(stderr, fg(fmt::color::red), "Player #{} kicked (not initialized)\n", it->index);
		}
	}
}

ENetPacket* build_game_data_packet(const GameData& gameData, const Player& targetPlayer)
{
	GameDataPacket packet;

	packet.targetPlayerIndex = targetPlayer.index;
	for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
	{
		if (it->peer == nullptr || it->IsPending())
			continue;

		GameDataPacket::PlayerPacketData player;
		player.index = it->index;
		player.name = it->name;
		player.ready = it->ready;

		packet.playerList.push_back(player);
	}

	return build_packet<GameDataPacket>(packet, ENET_PACKET_FLAG_RELIABLE);
}

ENetPacket* build_running_state_packet(const GameData& gameData)
{
	GameStateRunningPacket packet;
	for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
	{
		if (it->peer == nullptr || it->IsPending())
			continue;

		GameStateRunningPacket::RunningPacketData player;
		player.playerIndex = it->index;
		player.slotId = it->spawnSlotId;
		player.isInfected = it->isInfected;

		packet.playerList.push_back(player);
	}

	return build_packet<GameStateRunningPacket>(packet, ENET_PACKET_FLAG_RELIABLE);
}

ENetPacket* build_player_state_packet(const GameData& gameData, const Player& targetPlayer)
{
	if (targetPlayer.car == nullptr)
		throw std::runtime_error("Player doesn't have a car");

	PlayersStatePacket packet;
	for (const Player& other : gameData.players)
	{
		if (other.index == targetPlayer.index)
			continue;

		PlayersStatePacket::PlayerState playerState;
		playerState.playerIndex = other.index;
		playerState.inputs = other.lastInput;
		playerState.turnAngle = other.car->GetCurrentTurnAngle();

		playerState.position = other.car->GetPhysixActor().getGlobalPose().p;
		playerState.rotation = other.car->GetPhysixActor().getGlobalPose().q;

		playerState.linearVelocity = other.car->GetPhysixActor().getLinearVelocity();
		playerState.angularVelocity = other.car->GetPhysixActor().getAngularVelocity();


		playerState.atRest = playerState.linearVelocity.magnitude() <= 0.01f && playerState.angularVelocity.magnitude() <= 0.01f;
		if (!playerState.atRest)
		{
			playerState.frontLeftWheelVelocity = other.car->GetFrontLeftWheelVelocity();
			playerState.frontRightWheelVelocity = other.car->GetFrontRightWheelVelocity();
			playerState.rearLeftWheelVelocity = other.car->GetRearLeftWheelVelocity();
			playerState.rearRightWheelVelocity = other.car->GetRearRightWheelVelocity();
		}

		packet.otherPlayersState.push_back(playerState);
	}

	packet.inputIndex = targetPlayer.lastInputIndex;
	packet.localTurnAngle = targetPlayer.car->GetCurrentTurnAngle();

	packet.localPosition = targetPlayer.car->GetPhysixActor().getGlobalPose().p;
	packet.localRotation = targetPlayer.car->GetPhysixActor().getGlobalPose().q;

	packet.localLinearVelocity = targetPlayer.car->GetPhysixActor().getLinearVelocity();
	packet.localAngularVelocity = targetPlayer.car->GetPhysixActor().getAngularVelocity();

	//fmt::println("Linear velocity : x {}, y {}, z {},    angular velocity : x {}, y {}, z {}", packet.localLinearVelocity.x, packet.localLinearVelocity.y, packet.localLinearVelocity.z, packet.localAngularVelocity.x, packet.localAngularVelocity.y, packet.localAngularVelocity.z);

	packet.localAtRest = packet.localLinearVelocity.magnitude() <= 0.01f && packet.localAngularVelocity.magnitude() <= 0.01f;
	if (!packet.localAtRest)
	{
		packet.localFrontLeftWheelVelocity = targetPlayer.car->GetFrontLeftWheelVelocity();
		packet.localFrontRightWheelVelocity = targetPlayer.car->GetFrontRightWheelVelocity();
		packet.localRearLeftWheelVelocity = targetPlayer.car->GetRearLeftWheelVelocity();
		packet.localRearRightWheelVelocity = targetPlayer.car->GetRearRightWheelVelocity();
	}

	return build_packet<PlayersStatePacket>(packet, 0);
}

void tick_physics(GameData& gameData, Map& map, float deltaTime)
{
	if (gameData.state == GameState::LOBBY)
		return;

	for (Player& player : gameData.players)
	{
		if (!player.inputBuffer.empty())
		{
			PlayerInputPacket& bufInput = player.inputBuffer.front();
			player.lastInput = bufInput.inputs;
			player.lastInputIndex = bufInput.inputIndex;
			player.inputBuffer.pop();
		}

		if(player.car != nullptr)
			player.car->UpdatePhysics(player.lastInput, deltaTime);
	}

	map.SimulatePhysics(deltaTime);

	for (const Player& player : gameData.players)
	{
		if (player.peer != nullptr && !player.IsPending())
		{
			ENetPacket* packet = build_player_state_packet(gameData, player);
			enet_peer_send(player.peer, 0, packet);
		}
	}
}

void tick_logic(GameData& gameData, Map& map, std::uint32_t now, const Command& cmdPrompt)
{
	switch (gameData.state)
	{
	case GameState::WAITING_GAME_START:
	{

		if (gameData.timer >= gameData.endTimer)
		{
			cmdPrompt.ClearLastPrompt();
			GameStateStartMovePacket moveStatePacket;
			if (!gameData.waitingStateInit)
			{
				fmt::print(stderr, fg(fmt::color::yellow), "INFO :");
				fmt::println(" Survivors can move");

				// send move not infected
				moveStatePacket.moveInfected = false;
				gameData.waitingStateInit = true;
				gameData.timer = now;
				gameData.endTimer = gameData.timer + WaitAfterSurvivorMove * 1000;

				ENetPacket* enetPacket = build_packet<GameStateStartMovePacket>(moveStatePacket, ENET_PACKET_FLAG_RELIABLE);
				for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
				{
					if (it->peer != nullptr)
						enet_peer_send(it->peer, 0, enetPacket);
				}
			}
			else
			{
				fmt::print(stderr, fg(fmt::color::yellow), "INFO :");
				fmt::println(" Infected can move");
				fmt::print(stderr, fg(fmt::color::yellow), "INFO :");
				fmt::println(" Game counter set up for {}s", GameDuration);

				// send move Infected
				moveStatePacket.moveInfected = true;
				gameData.state = GameState::GAME_STARTED;
				gameData.timer = now;
				gameData.endTimer = gameData.timer + GameDuration * 1000;

				GameStateStartPacket gameStartPacket;
				gameStartPacket.gameDuration = GameDuration * 1000;

				ENetPacket* enetPacket = build_packet<GameStateStartMovePacket>(moveStatePacket, ENET_PACKET_FLAG_RELIABLE);
				ENetPacket* enetPacketBis = build_packet<GameStateStartPacket>(gameStartPacket, ENET_PACKET_FLAG_RELIABLE);
				for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
				{
					if (it->peer != nullptr)
					{
						enet_peer_send(it->peer, 0, enetPacket);
						enet_peer_send(it->peer, 0, enetPacketBis);
					}
				}
			}
			cmdPrompt.RecoverLastPrompt();

		}
		else
			gameData.timer = now;
	}
		break;

	case GameState::GAME_STARTED:
		// Game counter
		if (gameData.timer >= gameData.endTimer)
		{
			gameData.lastGameInfectedWins = false;
			gameData.state = GameState::GAME_FINISHED;
		}
		else
			gameData.timer = now;

		break;

	case GameState::GAME_FINISHED:
	{
		GameStateFinishPacket gameFinishPacket;
		gameFinishPacket.infectedWins = gameData.lastGameInfectedWins;

		ENetPacket* enetPacket = build_packet<GameStateFinishPacket>(gameFinishPacket, ENET_PACKET_FLAG_RELIABLE);
		for (const Player& player : gameData.players)
		{
			if (player.peer != nullptr && !player.IsPending())
				enet_peer_send(player.peer, 0, enetPacket);
		}

		cmdPrompt.ClearLastPrompt();
		fmt::println("------------------ GAME ENDED ------------------");
		if(gameData.lastGameInfectedWins)
			fmt::println("INFECTED WINS");
		else
			fmt::println("SURVIVORS WINS");
		cmdPrompt.RecoverLastPrompt();

		map.Clear(gameData);

		for (Player& player : gameData.players)
		{
			player.isInfected = false;
			player.ready = false;
		}

		gameData.waitingStateInit = false;
		gameData.state = GameState::LOBBY;
	}
		break;
	}
}
