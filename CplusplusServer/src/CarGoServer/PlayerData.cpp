#include "CarGoServer/PlayerData.hpp"

bool Player::IsPending() const
{ 
	return name.empty(); 
}