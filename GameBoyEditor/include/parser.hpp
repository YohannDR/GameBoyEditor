#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "core.hpp"
#include "room.hpp"

struct Tilemap
{
    uint8_t width;
    uint8_t height;
    std::vector<uint8_t> data;

    constexpr Tilemap() = default;
    explicit constexpr Tilemap(const int32_t w, const int32_t h, const std::vector<uint8_t>& d)
        : width(static_cast<uint8_t>(w)), height(static_cast<uint8_t>(h)), data(d) {}
};

class Parser
{
    STATIC_CLASS(Parser)

public:
    static bool_t ParseProject();

    static bool_t ParseGraphicsArray(std::ifstream& file, std::string& line);
    static bool_t ParseRoomInfo(std::ifstream& file, std::string& line);

    static Palette ParsePalette(const std::string& pal);

    static inline std::unordered_map<std::string, std::vector<uint8_t>> graphics;
    static inline std::unordered_map<std::string, Tilemap> tilemaps;
    static inline std::unordered_map<std::string, std::vector<uint8_t>> clipdata;
    static inline std::vector<Room> rooms;

private:
    static bool_t ParseFileContents(const std::filesystem::path& filePath);
};
