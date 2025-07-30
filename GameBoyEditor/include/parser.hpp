#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "animation.hpp"
#include "core.hpp"
#include "room.hpp"

using Tilemap = std::vector<std::vector<uint8_t>>;
using Graphics = std::vector<uint8_t>;

enum class SymbolType : uint8_t
{
    Graphics,
    Tilemap,
    Clipdata,
    SpriteData,
    RoomData
};

class Parser
{
    STATIC_CLASS(Parser)

public:
    static bool_t ParseProject();

    static inline std::unordered_map<std::string, Graphics> graphics;
    static inline std::unordered_map<std::string, Tilemap> tilemaps;
    static inline std::unordered_map<std::string, std::vector<uint8_t>> clipdata;
    static inline std::vector<Room> rooms;
    static inline std::unordered_map<std::string, std::vector<SpriteData>> sprites;
    static inline std::unordered_map<std::string, Animation> animations;

    static inline std::unordered_map<std::string, std::vector<std::pair<SymbolType, std::string>>> fileAssociations;

    static inline std::vector<std::string> spriteIds;
    static inline std::vector<std::string> clipdataNames;

private:
    static bool_t ParseFileContents(const std::filesystem::path& filePath);

    static bool_t ParseGraphicsArray(std::ifstream& file, const std::filesystem::path& filePath, std::string& line);
    static bool_t ParseRoomInfo(std::ifstream& file, const std::filesystem::path& filePath, std::string& line);
    static bool_t ParseSpriteInfo(std::ifstream& file, const std::filesystem::path& filePath, std::string& line);
    static bool_t ParseAnimation(std::ifstream& file, const std::filesystem::path& path, std::string& line);

    static void ParseEnums();

    static Palette ParsePalette(const std::string& pal);
};
