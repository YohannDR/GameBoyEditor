#include "parser.hpp"

#include <fstream>
#include <iostream>

#include "application.hpp"
#include "GLFW/glfw3native.h"

bool_t Parser::ParseProject()
{
    const std::filesystem::path path = Application::projectPath;
    const std::filesystem::path srcPath = path / "src";

    for (const std::filesystem::directory_entry& dirEntry : std::filesystem::recursive_directory_iterator(srcPath))
    {
        // Skip folders
        if (dirEntry.is_directory())
            continue;

        const std::filesystem::path& file = dirEntry.path();
        // Skip non .c file
        if (file.extension() != ".c")
            continue;

        if (!ParseFileContents(file))
            return false;
    }

    ParseEnums();

    return true;
}

bool_t Parser::ParseGraphicsArray(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
{
    SymbolType type;
    int32_t width = 0;
    int32_t height = 0;

    const size_t idx = line.find('[');
    const std::string symbolName = line.substr(sizeof("const u8"), idx - sizeof("const u8"));

    if (symbolName.contains("Graphics"))
    {
        std::getline(file, line);
        std::getline(file, line);
        type = SymbolType::Graphics;
    }
    else if (symbolName.contains("Tilemap"))
    {
        std::getline(file, line);
        (void)sscanf_s(line.c_str(), "%d, %d", &width, &height);
        
        std::getline(file, line);
        type = SymbolType::Tilemap;
    }
    else if (symbolName.contains("Clipdata"))
    {
        std::getline(file, line);
        (void)sscanf_s(line.c_str(), "%d, %d", &width, &height);
        std::getline(file, line);
        type = SymbolType::Clipdata;
    }
    else
    {
        return false;
    }

    std::vector<uint8_t> data;

    while (true)
    {
        std::getline(file, line);
        if (line[0] == '}')
            break;

        if (type == SymbolType::Graphics)
        {
            int32_t buffer[16];
            (void)sscanf_s(line.c_str(), "%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",
                &buffer[0],  &buffer[1],  &buffer[2],  &buffer[3],
                &buffer[4],  &buffer[5],  &buffer[6],  &buffer[7],
                &buffer[8],  &buffer[9],  &buffer[10], &buffer[11],
                &buffer[12], &buffer[13], &buffer[14], &buffer[15]
            );
            data.append_range(buffer);
        }
        else if (type == SymbolType::Tilemap || type == SymbolType::Clipdata)
        {
            int32_t buffer[20];
            (void)sscanf_s(line.c_str(), "%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",
                &buffer[0],  &buffer[1],  &buffer[2],  &buffer[3],  &buffer[4],
                &buffer[5],  &buffer[6],  &buffer[7],  &buffer[8],  &buffer[9],
                &buffer[10], &buffer[11], &buffer[12], &buffer[13], &buffer[14],
                &buffer[15], &buffer[16], &buffer[17], &buffer[18], &buffer[19]
            );
            data.append_range(buffer);
        }
    }

    if (type == SymbolType::Graphics)
    {
        graphics[symbolName] = data;
    }
    else if (type == SymbolType::Tilemap)
    {
        tilemaps[symbolName].resize(height);

        const size_t h = static_cast<size_t>(height);
        for (size_t i = 0; i < h; i++)
        {
            const Tilemap::difference_type index = width * i;
            tilemaps[symbolName][i] = std::vector(data.begin() + index, data.begin() + index + width);
        }
    }
    else if (type == SymbolType::Clipdata)
    {
        // m_Clipdata[symbolName] = { width, height, data };
    }

    fileAssociations[filePath.string()].emplace_back(type, symbolName);

    return true;
}

bool_t Parser::ParseRoomInfo(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
{
    while (true)
    {
        std::getline(file, line);
        if (line[0] == '}')
            break;

        if (line.contains('['))
            continue;

        const std::string gfx = line.substr(sizeof("        .graphics = ") - 1, line.find(',') - sizeof("        .graphics = ") + 1);
        std::getline(file, line);
        const std::string tilemap = line.substr(sizeof("        .tilemap = ") - 1, line.find(',') - sizeof("        .tilemap = ") + 1);
        std::getline(file, line);
        const std::string clip = line.substr(sizeof("        .clipdata = ") - 1, line.find(',') - sizeof("        .clipdata = ") + 1);
        std::getline(file, line);
        const std::string palette = line.substr(sizeof("        .bgPalette = ") - 1, line.find(')') - sizeof("        .bgPalette = ") + 1);
        std::getline(file, line);
        const std::string spriteData = line.substr(sizeof("        .spriteData = ") - 1, line.find(',') - sizeof("        .spriteData = ") + 1);

        // Skip the line with },
        std::getline(file, line);

        rooms.emplace_back(gfx, tilemap, clip, ParsePalette(palette), spriteData);
    }

    fileAssociations[filePath.string()].emplace_back(SymbolType::RoomData, "sRooms");

    return true;
}

bool_t Parser::ParseSpriteInfo(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
{
    const size_t idx = line.find('[');
    const std::string symbolName = line.substr(sizeof("const struct RoomSprite"), idx - sizeof("const struct RoomSprite"));

    while (true)
    {
        std::getline(file, line);

        if (line.contains("ROOM_SPRITE_TERMINATOR"))
            break;

        if (line.contains('['))
            continue;

        int32_t x;
        int32_t y;
        int32_t part;

        (void)sscanf_s(line.c_str(), "        .x = %d,", &x);
        std::getline(file, line);
        (void)sscanf_s(line.c_str(), "        .y = %d,", &y);
        std::getline(file, line);
        const std::string type = line.substr(sizeof("        .id = ") - 1, line.find(',') - sizeof("        .id = ") + 1);
        std::getline(file, line);
        (void)sscanf_s(line.c_str(), "        .part = %d,", &part);
        // Skip the line with },
        std::getline(file, line);

        sprites[symbolName].emplace_back(x, y, type, part);
    }

    fileAssociations[filePath.string()].emplace_back(SymbolType::SpriteData, symbolName);

    return true;
}

bool_t Parser::ParseAnimation(std::ifstream& file, const std::filesystem::path& path, std::string& line)
{
    Animation animation;
    int32_t partCount;
    size_t animationFrame = 0;

    // Parse frames
    while (true)
    {
        if (line.contains("struct AnimData"))
            break;

        (void)sscanf_s(&line.c_str()[line.find('[') + 1], "OAM_DATA_SIZE(%d)] = {", &partCount);

        animation.emplace_back();

        // Consume the count in the array itself
        std::getline(file, line);

        for (int32_t i = 0; i < partCount; i++)
        {
            int32_t y;
            int32_t x;
            int32_t tileId;
            int32_t props;

            std::getline(file, line);
            (void)sscanf_s(line.c_str(), "    OAM_POS(%d), OAM_POS(%d), %d, %d,", &y, &x, &tileId, &props);

            animation[animationFrame].oam.emplace_back(y, x, tileId, props);
        }

        animationFrame++;
        // Consume };
        std::getline(file, line);
        // And the empty line
        std::getline(file, line);
        std::getline(file, line);
    }

    animationFrame = 0;

    const size_t idx = line.find('[');
    const std::string symbolName = line.substr(sizeof("static const struct AnimData"), idx - sizeof("static const struct AnimData"));

    // Parse animation data
    while (true)
    {
        // Get [X] = {
        std::getline(file, line);
        if (line.contains("SPRITE_ANIM_TERMINATOR"))
            break;

        // Consume oamPointer = 
        std::getline(file, line);

        int32_t duration;
        std::getline(file, line);
        (void)sscanf_s(line.c_str(),"        .duration = %d", &duration);

        animation[animationFrame].duration = static_cast<uint8_t>(duration);
        // Consume },
        std::getline(file, line);

        animationFrame++;
    }

    animations[symbolName] = animation;

    return true;
}

void Parser::ParseEnums()
{
    // Look for specific enums in specific files

    const std::filesystem::path path = Application::projectPath;
    std::filesystem::path filePath = path / "include/sprite.h";

    std::ifstream file;
    file.open(filePath);
    std::string line;
    bool_t parsing = false;

    while (file)
    {
        std::getline(file, line);

        if (line.contains("enum SpriteType"))
        {
            parsing = true;
            continue;
        }

        if (!parsing)
            continue;

        if (line.contains("STYPE_END"))
        {
            parsing = false;
            break;
        }

        if (line.contains("STYPE"))
        {
            const std::string name = line.substr(sizeof("    ") - 1, line.find(',') - sizeof("    ") + 1);
            spriteIds.push_back(name);
        }
    }

    file.close();

    filePath = path / "include/bg_clip.h";

    file.open(filePath);
    while (file)
    {
        std::getline(file, line);

        if (line.contains("enum ClipdataValue"))
        {
            parsing = true;
            continue;
        }

        if (!parsing)
            continue;

        if (line.contains("CLIPDATA_END"))
            break;

        if (line.contains("CLIPDATA"))
        {
            const std::string name = line.substr(sizeof("    ") - 1, line.find(',') - sizeof("    ") + 1);
            clipdataNames.push_back(name);
        }
    }

    file.close();
}

Palette Parser::ParsePalette(const std::string& pal)
{
    Palette palette;

    const size_t i0 = pal.find(',', sizeof("MAKE_PALETTE("));
    const std::string c0 = pal.substr(sizeof("MAKE_PALETTE(") - 1, i0 - sizeof("MAKE_PALETTE(") + 1);

    const size_t i1 = pal.find(',', i0 + 1);
    const std::string c1 = pal.substr(i0 + 2, i1 - i0 - 2);

    const size_t i2 = pal.find(',', i1 + 1);
    const std::string c2 = pal.substr(i1 + 2, i2 - i1 - 2);

    const size_t i3 = pal.find(',', i2 + 1);
    const std::string c3 = pal.substr(i2 + 2, i3 - i2 - 2);

    palette[0] = GetColorFromString(c0);
    palette[1] = GetColorFromString(c1);
    palette[2] = GetColorFromString(c2);
    palette[3] = GetColorFromString(c3);

    return palette;
}

bool_t Parser::ParseFileContents(const std::filesystem::path& filePath)
{
    std::ifstream file;

    file.open(filePath);

    if (file.bad())
        return false;

    std::string line;
    while (file)
    {
        std::getline(file, line);

        if (line.starts_with("const u8 "))
        {
            ParseGraphicsArray(file, filePath, line);
        }
        else if (line.starts_with("const struct RoomInfo"))
        {
            ParseRoomInfo(file, filePath, line);
        }
        else if (line.starts_with("const struct RoomSprite"))
        {
            ParseSpriteInfo(file, filePath, line);
        }
        else if (line.starts_with("static const u8") && line.contains("_Frame"))
        {
            ParseAnimation(file, filePath, line);
        }
    }

    file.close();
    return true;
}
