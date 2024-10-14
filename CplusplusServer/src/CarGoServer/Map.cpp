#include "CarGoServer/Constant.hpp"
#include "CarGoServer/Map.hpp"

Map::Map()
{
	// init physicx
	// load map
}

Map::~Map()
{
	// unload physicx
}

void Map::UpdatePhisics()
{
	// update all cars
}

void Map::Clear()
{
	// remove all cars
}

void Map::InitPlayers(const GameData& gameData)
{
	// determine who is infected and who's not


	// add cars dynamic body (by static function of ClientCar(int spawnId, bool isInfected))
	// store in a list
}