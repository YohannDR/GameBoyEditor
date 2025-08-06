#pragma once

#include <string>

#include "color.hpp"

struct Room
{
    std::string tilemap;
    Palette colorPalette;
    std::string spriteData;
    std::string doorData;
    uint8_t collisionTable;
    uint16_t originX;
    uint16_t originY;
};

struct SpriteData
{
    uint8_t x;
    uint8_t y;
    std::string id;
    uint8_t part;

    constexpr SpriteData(const uint8_t xPos, const uint8_t yPos, std::string spriteId, const uint8_t partIndex) :
        x(xPos), y(yPos), id(std::move(spriteId)), part(partIndex) {}

    _NODISCARD bool_t operator==(const SpriteData& other) const { return x == other.x && y == other.y && part == other.part && id == other.id; }
};
