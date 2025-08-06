#pragma once

#include "core.hpp"

struct Door
{
    uint16_t x;
    uint16_t y;
    uint8_t ownerRoom;
    uint8_t height;
    uint8_t width;
    uint8_t targetDoor;
    int8_t exitX;
    int8_t exitY;
    uint8_t tileset;

    _NODISCARD bool_t operator==(const Door& other) const
    {
        return x == other.x && y == other.y && ownerRoom == other.ownerRoom && height == other.height && width == other.width &&
            targetDoor == other.targetDoor && exitX == other.exitX && exitY == other.exitY && tileset == other.tileset;
    }
};
