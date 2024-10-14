#include "CarGoServer/Constant.hpp"
#include "CarGoServer/Protocol.hpp"
#include "CarGoServer/PlayerData.hpp"
#include "CarGoServer/GameData.hpp"
#include "CarGoServer/Command.hpp"
#include <fmt/core.h>
#include <fmt/color.h>
#include <enet6/enet.h>
#include <vector>
#include <memory>
#include <stdexcept>
#include <physx/PxPhysicsAPI.h>
#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <CarGoServer/MapData.hpp>
#include <CarGoServer/Map.hpp>
#include <CarGoServer/Map.hpp>

using namespace physx;

void handle_message(Player& player, const std::vector<std::uint8_t>& message, GameData& gameData);
void PurgePlayers(const GameData& gameData);
ENetPacket* build_game_data_packet(GameData gameData, const Player& targetPlayer);
ENetPacket* build_running_state_packet(GameData gameData);
void UnserializeMap(std::string mapPath);

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

	fmt::println("< ======================================== >");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "     ___ ___   _   _____   __\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "    | _ \\ __| /_\\ |   \\ \\ / /\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "    |   / _| / _ \\| |) \\ V /\n");
	fmt::print(stderr, fg(fmt::color::medium_spring_green), "    |_|_\\___/_/ \\_\\___/ |_|\n");
	fmt::println("Application port : {}\n", AppPort);

	Command cmdPrompt;

	GameData gameData;
	gameData.state = GameState::LOBBY;

	Map map;

	bool serverOpen = true;
	while (serverOpen)
	{
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
					}
					else
						fmt::println(" Player #{} disconnected (no name)", player.index);

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

					handle_message(player, content, gameData);
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
				PurgePlayers(gameData);
				break;

			case Command::Action::Kick:
			{
				auto it = std::find_if(gameData.players.begin(), gameData.players.end(), [&commandReport](const Player& other) {return commandReport->params == other.index; });
				if (it != gameData.players.end())
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

void handle_message(Player& player, const std::vector<std::uint8_t>& message, GameData& gameData)
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

			fmt::print(stderr, fg(fmt::color::green), "Player #{} (aka {}) successfully initialized\n", player.index, player.name);
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
				PurgePlayers(gameData);

				// init world

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

		// Traitement input
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

ENetPacket* build_game_data_packet(GameData gameData, const Player& targetPlayer)
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

ENetPacket* build_running_state_packet(GameData gameData)
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

		packet.playersState.push_back(player);
	}

	return build_packet<GameStateRunningPacket>(packet, ENET_PACKET_FLAG_RELIABLE);
}


void UnserializeMap(std::string mapPath) {
	std::ifstream file(mapPath);

	if (!file.is_open()) {
		std::cerr << "Erreur lors de l'ouverture du fichier : " << mapPath << std::endl;
		return;
	}	

	nlohmann::json data;
	try {
		data = nlohmann::json::parse(file);
	}
	catch (const std::exception& e) {
		std::cerr << "Erreur lors de l'analyse du fichier JSON: " << e.what() << std::endl;
		return;
	}

	//std::cout << "Donn�es JSON charg�es: " << data.dump(4) << std::endl; // Affichage de mani�re lisible

	MapData mapData;
	mapData.from_json(data);

	//// Optionnel: Affiche les objets physiques charg�s
	//for (const auto& obj : mapData.physicObjects) {
	//	std::cout << "Objet charg� de type: " << obj->Type << std::endl;
	
	for (const auto& obj : mapData.physicObjects)
	{
		if (obj) {  
			if (obj->Type == "Capsule") {
				CapsuleObject* capsule = dynamic_cast<CapsuleObject*>(obj.get());
				physx::PxCapsuleGeometry geometry = physx::PxCapsuleGeometry(physx::PxReal(capsule->radius), physx::PxReal(capsule->height / 2));
			}
			else if (obj->Type == "Sphere") {
				
			}
			else if (obj->Type == "Box"){

			}
			else if (obj->Type == "Mesh") {

			}
		}
	}
}