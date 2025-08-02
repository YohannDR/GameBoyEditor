#pragma once

#include <array>
#include <string>

#include "core.hpp"
#include "imgui/imgui.h"

enum Color : uint8_t
{
    White,
    LightGrey,
    DarkGrey,
    Black,
    Transparent
};

constexpr uint32_t GetRgbColor(const Color c)
{
    switch (c)
    {
        case White: return IM_COL32(0xFF, 0xFF, 0xFF, 0xFF);
        case LightGrey: return IM_COL32(0xC0, 0xC0, 0xC0, 0xFF);
        case DarkGrey: return IM_COL32(0x80, 0x80, 0x80, 0xFF);
        case Black: return IM_COL32(0x00, 0x00, 0x00, 0xFF);
        case Transparent: return IM_COL32(0x00, 0x00, 0x00, 0x00);
    }

    return IM_COL32(0x00, 0x00, 0x00, 0xFF);
}

constexpr ImVec4 GetRgbColorVec(const Color c)
{
    switch (c)
    {
        case White: return ImVec4(1, 1, 1, 1);
        case LightGrey: return ImVec4(.75, .75, .75, 1);
        case DarkGrey: return ImVec4(.5, .5, .5, 1);
        case Black: return ImVec4(0, 0, 0, 1);
        case Transparent: return ImVec4(0, 0, 0, 0);
    }

    return ImVec4(0, 0, 0, 1);
}

inline Color GetColorFromString(const std::string& color)
{
    if (color == "COLOR_WHITE")
        return White;
    if (color == "COLOR_LIGHT_GRAY")
        return LightGrey;
    if (color == "COLOR_DARK_GRAY")
        return DarkGrey;
    if (color == "COLOR_BLACK")
        return Black;

    return White;
}

using Palette = std::array<Color, 4>;
