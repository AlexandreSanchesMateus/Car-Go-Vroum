#include "CarGoServer/PlayerInput.hpp"
#include "CarGoServer/Protocol.hpp"

void PlayerInput::Serialize(std::vector<std::uint8_t>& byteArray) const
{
    std::uint8_t inputByte = 0;

    if (acceleration == 1)
        inputByte |= 0b1;
    else if (acceleration == -1)
        inputByte |= 0b10;

    if (steer == 1)
        inputByte |= 0b100;
    else if (steer == -1)
        inputByte |= 0b1000;

    if (brake)
        inputByte |= 0b10000;

    if (softRecover)
        inputByte |= 0b100000;

    Serialize_u8(byteArray, inputByte);
}

PlayerInput PlayerInput::Deserialize(const std::vector<std::uint8_t>& byteArray, std::size_t& offset)
{
	PlayerInput input;
    std::uint8_t inputByte = Deserialize_u8(byteArray, offset);

    if ((inputByte & 0b1) != 0)
        input.acceleration = 1;
    else if ((inputByte & 0b10) != 0)
        input.acceleration = -1;

    if ((inputByte & 0b100) != 0)
        input.steer = 1;
    else if ((inputByte & 0b1000) != 0)
        input.steer = -1;

    if ((inputByte & 0b10000) != 0)
        input.brake = true;

    if ((inputByte & 0b100000) != 0)
        input.softRecover = true;

	return input;
}
