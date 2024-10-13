#pragma once

#include "CarGoServer/GameData.hpp"


class Map
{
public:
	Map();
	~Map();

	Map(const Map&) = delete;
	Map(Map&&) = delete;

	Map& operator=(const Map&) = delete;
	Map& operator=(Map&&) = delete;

	void UpdatePhisics();
	void Clear();
	void InitPlayers(const GameData& gameData);

private:
	// MapData m_mapData;
};