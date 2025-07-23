#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "core.hpp"

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

    static inline std::unordered_map<std::string, std::vector<uint8_t>> graphics;
    static inline std::unordered_map<std::string, Tilemap> tilemaps;
    static inline std::unordered_map<std::string, std::vector<uint8_t>> clipdata;

private:
    static bool_t ParseFileContents(const std::filesystem::path& filePath);
};
