#pragma once

#include <array>

#include "core.hpp"
#include "imgui/imgui.h"

enum Color : uint8_t
{
    White,
    LightGrey,
    DarkGrey,
    Black
};

constexpr inline uint32_t GetRgbColor(const Color c)
{
    switch (c)
    {
        case White: return IM_COL32(0xFF, 0xFF, 0xFF, 0xFF);
        case LightGrey: return IM_COL32(0xC0, 0xC0, 0xC0, 0xFF);
        case DarkGrey: return IM_COL32(0x80, 0x80, 0x80, 0xFF);
        case Black: return IM_COL32(0x00, 0x00, 0x00, 0xFF);
    }

    return 0x000000FF;
}

using Palette = std::array<Color, 4>;
