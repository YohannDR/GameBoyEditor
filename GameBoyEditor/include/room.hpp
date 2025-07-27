#pragma once

#include <string>

#include "color.hpp"

struct Room
{
    std::string graphics;
    std::string tilemap;
    std::string clipdata;
    Palette colorPalette;
    std::string spriteData;
};

struct SpriteData
{
    uint8_t x;
    uint8_t y;
    std::string id;
    uint8_t part;

    constexpr SpriteData(const uint8_t xPos, const uint8_t yPos, std::string spriteId, const uint8_t partIndex) :
        x(xPos), y(yPos), id(std::move(spriteId)), part(partIndex) {}
};
