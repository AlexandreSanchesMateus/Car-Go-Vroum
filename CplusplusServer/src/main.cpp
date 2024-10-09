#include "CarGoServer/Constant.hpp"
#include "CarGoServer/Protocol.hpp"
#include "CarGoServer/PlayerData.hpp"
#include "CarGoServer/GameData.hpp"
#include <fmt/core.h>
#include <fmt/color.h>
#include <enet6/enet.h>
#include <vector>
#include <memory>
#include <stdexcept>


void handle_message(Player& player, const std::vector<std::uint8_t>& message, GameData& gameData);
ENetPacket* build_player_list_packet(GameData gameData);
ENetPacket* build_running_state_packet(GameData gameData);

int main(int argc, char* argv[])
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

	fmt::print(stderr, fg(fmt::color::green), "=> ENet Initialized\n");

	// Création de l'hôte serveur
	ENetAddress address;
	enet_address_build_any(&address, ENET_ADDRESS_TYPE_IPV6);
	address.port = AppPort;

	ENetHost* host = enet_host_create(ENET_ADDRESS_TYPE_ANY, &address, 10, 0, 0, 0);
	if (!host)
	{
		fmt::print(stderr, fg(fmt::color::red), "Failed to create ENet host\n");
		return EXIT_FAILURE;
	}

	fmt::print(stderr, fg(fmt::color::green), "=> ENet host created\n\n");

	fmt::print(stderr, fg(fmt::color::green), " ___ ___   _   _____   __\n");
	fmt::print(stderr, fg(fmt::color::green), "| _ \\ __| /_\\ |   \\ \\ / /\n");
	fmt::print(stderr, fg(fmt::color::green), "|   / _| / _ \\| |) \\ V /\n");
	fmt::print(stderr, fg(fmt::color::green), "|_|_\\___/_/ \\_\\___/ |_|\n");
	fmt::println("                 Application port : {}\n", AppPort);

	GameData gameData;
	gameData.state = GameState::WAITING;

	for (;;)
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

					fmt::println("Player #{} connected", player.index);
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

					fmt::println("Player #{} disconnected", player.index);

					if (!player.IsPending())
					{
						// envoyer aux joueur que ce joueur c'est déconnecté
					}
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
					fmt::print(stderr, fg(fmt::color::yellow), "Enet event type : unexpected event\n");
					break;
				}
			} while (enet_host_check_events(host, &event) > 0);
		}

		// Tick logique
	}

	/*for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
	{
		if (it->peer != nullptr)
			enet_peer_disconnect_now(it->peer, 0);
	}*/

	enet_deinitialize();

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

		if (gameData.state != GameState::WAITING || playerCount > MaxPlayerCount)
		{
			// Deconnection player


		}
		else
		{
			// Init player
			GameDataPacket gameState;
			gameState.playerIndex = player.index;
			enet_peer_send(player.peer, 0, build_packet<GameDataPacket>(gameState, ENET_PACKET_FLAG_RELIABLE));

			// Broadcast
			ENetPacket* playerList = build_player_list_packet(gameData);
			for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
			{
				enet_peer_send(it->peer, 0, playerList);
			}
		}
	}
		break;

	case Opcode::C_PlayerReady:
	{
		PlayerReadyPacket playerReady = PlayerReadyPacket::Deserialize(message, offset);

		player.ready = playerReady.ready;

		// vérifier si le jeu n'a pas commencer
		if (gameData.state == GameState::WAITING)
		{
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
				// init world

				ENetPacket* packet = build_running_state_packet(gameData);
				for (const Player& player : gameData.players)
				{
					if (player.peer != nullptr && !player.IsPending())
						enet_peer_send(player.peer, 0, packet);
				}
			}
		}
	}
		break;

	case Opcode::C_PlayerInputs:
		PlayerInputPacket playerInputs = PlayerInputPacket::Deserialize(message, offset);

		// Traitement input

		break;
	}
}

ENetPacket* build_player_list_packet(GameData gameData)
{
	PlayerListPacket packet;
	for (std::vector<Player>::const_iterator it = gameData.players.begin(); it != gameData.players.end(); ++it)
	{
		if (it->peer == nullptr || it->IsPending())
			continue;

		PlayerListPacket::PlayerPacketData player;
		player.index = it->index;
		player.name = it->name;
		player.ready = it->ready;

		packet.playerList.push_back(player);
	}

	return build_packet<PlayerListPacket>(packet, ENET_PACKET_FLAG_RELIABLE);
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