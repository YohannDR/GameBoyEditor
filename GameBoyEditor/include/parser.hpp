#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "animation.hpp"
#include "core.hpp"
#include "door.hpp"
#include "room.hpp"

using Tilemap = std::vector<std::vector<uint8_t>>;
using Graphics = std::vector<uint8_t>;
using DoorData = std::vector<uint8_t>;

enum class SymbolType : uint8_t
{
    Graphics,
    Tilemap,
    SpriteData,
    DoorData,
    Animation,
    RoomData,
    Doors
};

using SymbolInfo = std::pair<SymbolType, std::string>;

class Parser
{
    STATIC_CLASS(Parser)

public:
    static bool_t ParseProject();
    static bool_t Save();

    static void RegisterSymbol(const std::string& file, const std::string& symbolName, SymbolType type);
    _NODISCARD static size_t GetDoorId(const Door& door);
    static void DeleteDoor(const Door& door);

    static inline std::unordered_map<std::string, Graphics> graphics;
    static inline std::unordered_map<std::string, Tilemap> tilemaps;
    static inline std::vector<Room> rooms;
    static inline std::vector<Door> doors;
    static inline std::unordered_map<std::string, std::vector<SpriteData>> sprites;
    static inline std::unordered_map<std::string, DoorData> roomsDoorData;
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
    static bool_t ParseDoorData(std::ifstream& file, const std::filesystem::path& filePath, std::string& line);
    static bool_t ParseDoors(std::ifstream& file, const std::filesystem::path& filePath, std::string& line);

    static void ParseEnums();

    static Palette ParsePalette(const std::string& pal);
    static std::string MakePalette(const Palette& pal);

    static std::fstream RemoveExistingSymbol(std::fstream& file, const std::filesystem::path& fileName, const SymbolInfo& symbol);
    static void RemoveDuplicateIncludes(std::fstream& file, const std::filesystem::path& fileName);

    static void SaveGraphics(std::fstream& file, const std::string& symbolName);
    static void SaveTilemap(std::fstream& file, const std::string& symbolName);
    static void SaveSpriteData(std::fstream& file, const std::string& symbolName);
    static void SaveDoorData(std::fstream& file, const std::string& symbolName);
    static void SaveAnimation(std::fstream& file, const std::string& symbolName);
    static void SaveRoomData(std::fstream& file, const std::string& symbolName);
    static void SaveDoors(std::fstream& file, const std::string& symbolName);

    static std::string ToHex(size_t value);
};
