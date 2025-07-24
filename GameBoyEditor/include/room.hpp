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
