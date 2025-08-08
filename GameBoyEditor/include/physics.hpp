#pragma once
#include <cstdint>

struct Physics
{
    uint8_t xAcceleration;
    uint8_t xVelocityCap;
    int8_t yVelocityCap;
    uint8_t gravityUpwards;
    uint8_t gravityDownwards;
    int8_t jumpingVelocity;
};
