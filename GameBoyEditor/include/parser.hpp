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
    Animation,
    RoomData
};

using SymbolInfo = std::pair<SymbolType, std::string>;

class Parser
{
    STATIC_CLASS(Parser)

public:
    static bool_t ParseProject();
    static bool_t Save();

    static inline std::unordered_map<std::string, Graphics> graphics;
    static inline std::unordered_map<std::string, Tilemap> tilemaps;
    static inline std::unordered_map<std::string, Tilemap> clipdata;
    static inline std::vector<Room> rooms;
    static inline std::unordered_map<std::string, std::vector<SpriteData>> sprites;
    static inline std::unordered_map<std::string, Animation> animations;

    static inline std::unordered_map<std::string, std::vector<SymbolInfo>> fileAssociations;
    static inline std::vector<std::string> existingSymbols;

    static inline std::vector<std::string> spriteIds;
    static inline std::vector<std::string> clipdataNames;

private:
    static bool_t ParseFileContents(const std::filesystem::path& filePath);

    static bool_t ParseGraphicsArray(std::ifstream& file, const std::filesystem::path& filePath, std::string& line);
    static bool_t ParseRoomInfo(std::ifstream& file, const std::filesystem::path& filePath, std::string& line);
    static bool_t ParseSpriteInfo(std::ifstream& file, const std::filesystem::path& filePath, std::string& line);
    static bool_t ParseAnimation(std::ifstream& file, const std::filesystem::path& filePath, std::string& line);

    static void ParseEnums();

    static Palette ParsePalette(const std::string& pal);
    static std::string MakePalette(const Palette& pal);

    static std::fstream RemoveExistingSymbol(std::fstream& file, const std::filesystem::path& fileName, const SymbolInfo& symbol);

    static void SaveGraphics(std::fstream& file, const std::string& symbolName);
    static void SaveTilemap(std::fstream& file, const std::string& symbolName);
    static void SaveClipdata(std::fstream& file, const std::string& symbolName);
    static void SaveSpriteData(std::fstream& file, const std::string& symbolName);
    static void SaveAnimation(std::fstream& file, const std::string& symbolName);
    static void SaveRoomData(std::fstream& file, const std::string& symbolName);

    static std::string ToHex(size_t value);
};
