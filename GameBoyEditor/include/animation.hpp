#pragma once

#include <vector>
#include "core.hpp"

struct OamEntry
{
    int8_t y;
    int8_t x;
    uint8_t tileIndex;
    uint8_t properties;
};

struct AnimationFrame
{
    std::vector<OamEntry> oam; 
    uint8_t duration;
};

using Animation = std::vector<AnimationFrame>;
