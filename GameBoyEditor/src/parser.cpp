#include "parser.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <ranges>

#include "application.hpp"

#define TAB "    "

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

bool_t Parser::Save()
{
    for (const std::pair<const std::string, std::vector<SymbolInfo>>& association : fileAssociations)
    {
        std::fstream file;
        const std::filesystem::path filePath = association.first;
        file.open(filePath, std::ifstream::out | std::ifstream::in | std::ifstream::app);

        for (const SymbolInfo& symbolInfo : association.second)
            file = RemoveExistingSymbol(file, filePath, symbolInfo);

        std::filesystem::path onlyFilePath;
        std::filesystem::path temp = filePath;

        while (temp != Application::projectPath)
        {
            const std::filesystem::path name = temp.filename();
            if (name == "src")
                break;

            onlyFilePath = name / onlyFilePath;
            temp = temp.parent_path();
        }

        onlyFilePath = onlyFilePath.parent_path().replace_extension(".h");
        std::string includeFile = onlyFilePath.string();
        std::ranges::replace(includeFile, '\\', '/');

        file << "#include \"" << includeFile << "\"\n";

        for (const SymbolInfo& symbolInfo : association.second)
        {
            switch (symbolInfo.first)
            {
                case SymbolType::Graphics: SaveGraphics(file, symbolInfo.second); break;
                case SymbolType::Tilemap: SaveTilemap(file, symbolInfo.second); break;
                case SymbolType::SpriteData: SaveSpriteData(file, symbolInfo.second); break;
                case SymbolType::DoorData: SaveDoorData(file, symbolInfo.second); break;
                case SymbolType::Animation: SaveAnimation(file, symbolInfo.second); break;
                case SymbolType::RoomData: SaveRoomData(file, symbolInfo.second); break;
                case SymbolType::Doors: SaveDoors(file, symbolInfo.second); break;
                case SymbolType::Tilesets: SaveTilesets(file, symbolInfo.second); break;
                case SymbolType::CollisionTable: SaveCollisionTable(file, symbolInfo.second); break;
                case SymbolType::CollisionTableArray: SaveCollisionTableArray(file, symbolInfo.second); break;
            }
        }

        RemoveDuplicateIncludes(file, filePath);

        file.close();
    }

    SavePhysics();

    return true;
}

void Parser::RegisterSymbol(const std::string& file, const std::string& symbolName, const SymbolType type)
{
    fileAssociations[file].emplace_back(type, symbolName);
    existingSymbols.push_back(symbolName);
}

size_t Parser::GetDoorId(const Door& door)
{
    return std::ranges::find(doors, door) - doors.begin();
}

void Parser::DeleteDoor(const Door& door)
{
    const size_t index = GetDoorId(door);
    std::erase(doors, door);

    for (DoorData& doorData : roomsDoorData | std::ranges::views::values)
    {
        for (size_t i = 0; i < doorData.size(); i++)  // NOLINT(modernize-loop-convert)
        {
            if (doorData[i] > index)
                doorData[i]--;
        }
    }

    for (Door& d : doors)
    {
        if (d.targetDoor > index)
            d.targetDoor--;
        else if (d.targetDoor == index)
            d.targetDoor = 0xFF; // TODO Maybe print a warning or something?
    }
}

void Parser::DeleteTileset(const size_t index)
{
    tilesets.erase(tilesets.begin() + static_cast<decltype(tilesets)::difference_type>(index));

    for (Door& door : doors)
    {
        if (door.tileset == 255)
            continue;

        if (door.tileset == index)
            door.tileset = 255;
        else if (door.tileset > index)
            door.tileset--;
    }
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

        if (line.contains("extern"))
            continue;

        if (line.starts_with("const u8 ") && (line.contains("Graphics") || line.contains("Tilemap")))
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
        else if (line.starts_with("const u8 ") && line.contains("DoorData"))
        {
            ParseDoorData(file, filePath, line);
        }
        else if (line.starts_with("const struct Door"))
        {
            ParseDoors(file, filePath, line);
        }
        else if (line.starts_with("const u8* const sTilesets"))
        {
            ParseTilesets(file, filePath, line);
        }
        else if (line.starts_with("const u8 sCollisionTable_"))
        {
            ParseCollisionTable(file, filePath, line);
        }
        else if (line.starts_with("const u8* const sCollisionTables"))
        {
            ParseCollisionTableArray(file, filePath, line);
        }
        else if (line.starts_with("static void PlayerInitPhysics"))
        {
            ParsePhysics(file, filePath, line);
        }
    }

    file.close();
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
        else if (type == SymbolType::Tilemap)
        {
            std::istringstream stream(line);

            for (size_t i = 0; i < width; i++)
            {
                int32_t v;
                char_t c;
                stream >> std::hex >> v >> c;
                data.push_back(v);
            }
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

    RegisterSymbol(filePath.string(), symbolName, type);

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

        const std::string tilemap = line.substr(sizeof("        .tilemap = ") - 1, line.find(',') - sizeof("        .tilemap = ") + 1);
        std::getline(file, line);
        const std::string palette = line.substr(sizeof("        .bgPalette = ") - 1, line.find(')') - sizeof("        .bgPalette = ") + 1);
        std::getline(file, line);
        const std::string spriteData = line.substr(sizeof("        .spriteData = ") - 1, line.find(',') - sizeof("        .spriteData = ") + 1);
        std::getline(file, line);
        const std::string doorData = line.substr(sizeof("        .doorData = ") - 1, line.find(',') - sizeof("        .doorData = ") + 1);
        std::getline(file, line);
        int32_t collisionTable;
        (void)sscanf_s(line.c_str(), "        .collisionTable = %d,", &collisionTable);
        std::getline(file, line);
        int32_t originX;
        (void)sscanf_s(line.c_str(), "        .originX = SCREEN_SIZE_X_SUB_PIXEL * %d,", &originX);
        std::getline(file, line);
        int32_t originY;
        (void)sscanf_s(line.c_str(), "        .originY = SCREEN_SIZE_Y_SUB_PIXEL * %d,", &originY);

        // Skip the line with },
        std::getline(file, line);

        rooms.emplace_back(tilemap, ParsePalette(palette), spriteData, doorData, collisionTable, originX, originY);
    }

    RegisterSymbol(filePath.string(), "sRooms", SymbolType::RoomData);

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

    RegisterSymbol(filePath.string(), symbolName, SymbolType::SpriteData);

    return true;
}

bool_t Parser::ParseAnimation(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
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
    const std::string symbolName = line.substr(sizeof("const struct AnimData"), idx - sizeof("const struct AnimData"));

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

    RegisterSymbol(filePath.string(), symbolName, SymbolType::Animation);

    return true;
}

bool_t Parser::ParseDoorData(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
{
    const size_t idx = line.find('[');
    const std::string symbolName = line.substr(sizeof("const u8"), idx - sizeof("const u8"));

    DoorData doorData;

    for (;;)
    {
        std::getline(file, line);

        if (line.contains("DOOR_NONE"))
            break;

        int32_t doorId;
        (void)sscanf_s(line.c_str(), "    %d,", &doorId);
        doorData.push_back(static_cast<uint8_t>(doorId));
    }

    roomsDoorData[symbolName] = doorData;
    RegisterSymbol(filePath.string(), symbolName, SymbolType::DoorData);

    return true;
}

bool_t Parser::ParseDoors(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
{
    while (true)
    {
        std::getline(file, line);
        if (line[0] == '}')
            break;

        if (line.contains('['))
            continue;

        int32_t x;
        (void)sscanf_s(line.c_str(), "        .x = %d,", &x);
        std::getline(file, line);
        int32_t y;
        (void)sscanf_s(line.c_str(), "        .y = %d,", &y);
        std::getline(file, line);
        int32_t ownerRoom;
        (void)sscanf_s(line.c_str(), "        .ownerRoom = %d,", &ownerRoom);
        std::getline(file, line);
        int32_t height;
        (void)sscanf_s(line.c_str(), "        .height = %d,", &height);
        std::getline(file, line);
        int32_t width;
        (void)sscanf_s(line.c_str(), "        .width = %d,", &width);
        std::getline(file, line);
        int32_t targetDoor;
        (void)sscanf_s(line.c_str(), "        .targetDoor = %d,", &targetDoor);
        std::getline(file, line);
        int32_t exitX;
        (void)sscanf_s(line.c_str(), "        .exitX = %d,", &exitX);
        std::getline(file, line);
        int32_t exitY;
        (void)sscanf_s(line.c_str(), "        .exitY = %d,", &exitY);
        std::getline(file, line);
        int32_t tileset;
        (void)sscanf_s(line.c_str(), "        .tileset = %d,", &tileset);

        // Skip the line with },
        std::getline(file, line);

        doors.emplace_back(x, y, ownerRoom, height, width, targetDoor, exitX, exitY, tileset);
    }

    RegisterSymbol(filePath.string(), "sDoors", SymbolType::Doors);
    return true;
}

bool_t Parser::ParseTilesets(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
{
    while (true)
    {
        std::getline(file, line);
        if (line[0] == '}')
            break;

        const std::string gfxName = line.substr(sizeof("    ") - 1, line.find(',') - sizeof("    ") + 1);
        tilesets.push_back(gfxName);
    }

    RegisterSymbol(filePath.string(), "sTilesets", SymbolType::Tilesets);
    return true;
}

bool_t Parser::ParseCollisionTable(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
{
    const size_t idx = line.find('[');
    const std::string symbolName = line.substr(sizeof("const u8"), idx - sizeof("const u8"));

    while (true)
    {
        std::getline(file, line);

        if (line[0] == '}')
            break;

        const std::string clipdata = line.substr(sizeof("    ") - 1, line.find(',') - sizeof("    ") + 1);
        collisionTables[symbolName].push_back(clipdata);
    }

    RegisterSymbol(filePath.string(), symbolName, SymbolType::CollisionTable);
    return true;
}

bool_t Parser::ParseCollisionTableArray(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
{
    while (true)
    {
        std::getline(file, line);

        if (line[0] == '}')
            break;

        const std::string clipdata = line.substr(sizeof("    ") - 1, line.find(',') - sizeof("    ") + 1);
        collisionTableArray.push_back(clipdata);
    }

    RegisterSymbol(filePath.string(), "sCollisionTables", SymbolType::CollisionTableArray);
    return true;
}

bool_t Parser::ParsePhysics(std::ifstream& file, const std::filesystem::path& filePath, std::string& line)
{
    // {
    std::getline(file, line);

    std::getline(file, line);
    (void)sscanf_s(line.c_str(), "    gPlayerPhysics.xAcceleration = %hhu", &physics.xAcceleration);
    std::getline(file, line);
    (void)sscanf_s(line.c_str(), "    gPlayerPhysics.xVelocityCap = %hhu", &physics.xVelocityCap);
    std::getline(file, line);
    (void)sscanf_s(line.c_str(), "    gPlayerPhysics.yVelocityCap = %hhd", &physics.yVelocityCap);
    std::getline(file, line);
    (void)sscanf_s(line.c_str(), "    gPlayerPhysics.gravityUpwards = %hhu", &physics.gravityUpwards);
    std::getline(file, line);
    (void)sscanf_s(line.c_str(), "    gPlayerPhysics.gravityDownwards = %hhu", &physics.gravityDownwards);
    std::getline(file, line);
    (void)sscanf_s(line.c_str(), "    gPlayerPhysics.jumpingVelocity = %hhd", &physics.jumpingVelocity);

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

std::string Parser::MakePalette(const Palette& pal)
{
    const std::string colorNames[] = {
        "COLOR_WHITE",
        "COLOR_LIGHT_GRAY",
        "COLOR_DARK_GRAY",
        "COLOR_BLACK"
    };

    std::string result = "MAKE_PALETTE(";

    result += colorNames[pal[0]];
    result += ", ";
    result += colorNames[pal[1]];
    result += ", ";
    result += colorNames[pal[2]];
    result += ", ";
    result += colorNames[pal[3]];
    result += ')';

    return result;
}

std::fstream Parser::RemoveExistingSymbol(std::fstream& file, const std::filesystem::path& fileName, const SymbolInfo& symbol)
{
    std::string line;
    bool_t deleting = false;
    size_t additionalLinesToDelete = 0;

    std::vector<std::string> lines;

    while (file)
    {
        std::getline(file, line);

        if (line.contains(symbol.second) && line.contains("const") && !line.contains("extern"))
        {
            deleting = true;
            continue;
        }

        if (deleting)
        {
            const char_t* const terminator = symbol.first == SymbolType::Animation ? "SPRITE_ANIM_TERMINATOR" : "};";
            if (line.contains(terminator))
            {
                deleting = false;

                if (symbol.first == SymbolType::Animation)
                {
                    // Need to consume the }; and the empty line afterwards
                    additionalLinesToDelete = 2;
                }
                else
                {
                    // Also need to consume the empty line afterwards
                    additionalLinesToDelete = 1;
                }
            }
        }
        else
        {
            if (additionalLinesToDelete != 0)
                additionalLinesToDelete--;
            else
                lines.push_back(line);
        }
    }

    std::fstream tempFile;
    tempFile.open("temp.txt", std::fstream::out | std::fstream::ate);
    for (size_t i = 0; i < lines.size() - 1; i++)
        tempFile << lines[i] << '\n';

    tempFile.close();
    file.close();

    (void)remove(fileName.string().c_str());
    (void)rename("temp.txt", fileName.string().c_str());

    tempFile.open(fileName, std::ifstream::out | std::ifstream::in | std::ifstream::app);
    return tempFile;
}

void Parser::RemoveDuplicateIncludes(std::fstream& file, const std::filesystem::path& fileName)
{
    file.clear();
    file.seekg(0);
    std::string line;
    std::vector<std::string> currentIncludes;
    std::vector<std::string> lines;

    while (file)
    {
        std::getline(file, line);

        if (line.contains("include"))
        {
            if (!std::ranges::contains(currentIncludes, line))
            {
                lines.push_back(line);
                currentIncludes.push_back(line);
            }
        }
        else
        {
            lines.push_back(line);
        }
    }

    std::fstream tempFile;
    tempFile.open("temp.txt", std::fstream::out | std::fstream::ate);
    for (size_t i = 0; i < lines.size() - 1; i++)
        tempFile << lines[i] << '\n';

    tempFile.close();
    file.close();

    (void)remove(fileName.string().c_str());
    (void)rename("temp.txt", fileName.string().c_str());
}

void Parser::SaveGraphics(std::fstream& file, const std::string& symbolName)
{
    file << "\nconst u8 " << symbolName << "[] = {\n";

    const Graphics& gfx = graphics[symbolName];
    const size_t tileAmount = gfx.size() / 16;

    file << TAB << tileAmount << ",\n\n";

    for (size_t i = 0; i < tileAmount; i++)
    {
        file << TAB;
        for (size_t j = 0; j < 16; j++)
        {
            file << ToHex(gfx[i * 16 + j]) << ',';
            file << (j == 15 ? '\n' : ' ');
        }
    }

    file << "};\n";
}

void Parser::SaveTilemap(std::fstream& file, const std::string& symbolName)
{
    file << "\nconst u8 " << symbolName << "[] = {\n";

    const Tilemap& tilemap = tilemaps[symbolName];

    file << TAB << tilemap[0].size() << ", " << tilemap.size() << ",\n\n";

    for (const std::vector<uint8_t>& row : tilemap)
    {
        file << TAB;
        for (size_t j = 0; j < row.size(); j++)
        {
            file << ToHex(row[j]) << ',';
            if (j == row.size() - 1)
                file << '\n';
        }
    }

    file << "};\n";
}

void Parser::SaveSpriteData(std::fstream& file, const std::string& symbolName)
{
    file << "\nconst struct RoomSprite " << symbolName << "[] = {\n";

    const std::vector<SpriteData>& spriteData = sprites[symbolName];

    for (size_t i = 0; i < spriteData.size(); i++)
    {
        file << TAB "[" << i << "] = {\n";
        file << TAB TAB ".x = " << static_cast<size_t>(spriteData[i].x) << ",\n";
        file << TAB TAB ".y = " << static_cast<size_t>(spriteData[i].y) << ",\n";
        file << TAB TAB ".id = " << spriteData[i].id << ",\n";
        file << TAB TAB ".part = " << static_cast<size_t>(spriteData[i].part) << '\n';
        file << TAB "},\n";
    }

    file << TAB "[" << spriteData.size() << "] = ROOM_SPRITE_TERMINATOR\n};\n";
}

void Parser::SaveDoorData(std::fstream& file, const std::string& symbolName)
{
    file << "\nconst u8 " << symbolName << "[] = {\n";

    const DoorData& doorData = roomsDoorData[symbolName];

    for (const uint8_t doorId : doorData)
        file << TAB << static_cast<size_t>(doorId) << ",\n";

    file << TAB << "DOOR_NONE\n};\n";
}

void Parser::SaveAnimation(std::fstream& file, const std::string& symbolName)
{
    const Animation& animation = animations[symbolName];

    for (size_t i = 0; i < animation.size(); i++)
    {
        const size_t oamCount = animation[i].oam.size();

        file << "\nstatic const u8 " << symbolName << "_Frame" << i << "[OAM_DATA_SIZE(" << oamCount << ")] = {\n";
        file << TAB << oamCount << ",\n";

        for (size_t j = 0; j < oamCount; j++)
        {
            file << TAB << "OAM_POS(" << static_cast<int32_t>(animation[i].oam[j].y) << "), ";
            file << "OAM_POS(" << static_cast<int32_t>(animation[i].oam[j].x) << "), ";
            file << static_cast<int32_t>(animation[i].oam[j].tileIndex) << ", ";
            file << static_cast<int32_t>(animation[i].oam[j].properties) << ",\n";
        }

        file << "};\n";
    }

    file << "\nconst struct AnimData " << symbolName << "[] = {\n";
    
    for (size_t i = 0; i < animation.size(); i++)
    {
        file << TAB << '[' << i << "] = {\n";
        file << TAB TAB << ".oamPointer = " << symbolName << "_Frame" << i << ",\n";
        file << TAB TAB << ".duration = " << static_cast<size_t>(animation[i].duration) << ",\n";
        file << TAB "},\n";
    }

    file << TAB << '[' << animation.size() << "] = SPRITE_ANIM_TERMINATOR\n";
    file << "};\n";
}

void Parser::SaveRoomData(std::fstream& file, const std::string& symbolName)
{
    file << "\nconst struct RoomInfo " << symbolName << "[] = {\n";

    for (size_t i = 0; i < rooms.size(); i++)
    {
        file << TAB "[" << i << "] = {\n";
        file << TAB TAB ".tilemap = " << rooms[i].tilemap << ",\n";
        file << TAB TAB ".bgPalette = " << MakePalette(rooms[i].colorPalette) << ",\n";
        file << TAB TAB ".spriteData = " << rooms[i].spriteData << ",\n";
        file << TAB TAB ".doorData = " << rooms[i].doorData << ",\n";
        file << TAB TAB ".collisionTable = " << static_cast<size_t>(rooms[i].collisionTable) << ",\n";
        file << TAB TAB ".originX = SCREEN_SIZE_X_SUB_PIXEL * " << static_cast<size_t>(rooms[i].originX) << ",\n";
        file << TAB TAB ".originY = SCREEN_SIZE_Y_SUB_PIXEL * " << static_cast<size_t>(rooms[i].originY) << ",\n";
        file << TAB "},\n";
    }

    file << "};\n";
}

void Parser::SaveDoors(std::fstream& file, const std::string& symbolName)
{
    file << "\nconst struct Door " << symbolName << "[] = {\n";

    for (size_t i = 0; i < doors.size(); i++)
    {
        file << TAB "[" << i << "] = {\n";
        file << TAB TAB ".x = " << doors[i].x << ",\n";
        file << TAB TAB ".y = " << doors[i].y << ",\n";
        file << TAB TAB ".ownerRoom = " << static_cast<size_t>(doors[i].ownerRoom) << ",\n";
        file << TAB TAB ".height = " << static_cast<size_t>(doors[i].height) << ",\n";
        file << TAB TAB ".width = " << static_cast<size_t>(doors[i].width) << ",\n";
        file << TAB TAB ".targetDoor = " << static_cast<size_t>(doors[i].targetDoor) << ",\n";
        file << TAB TAB ".exitX = " << static_cast<int32_t>(doors[i].exitX) << ",\n";
        file << TAB TAB ".exitY = " << static_cast<int32_t>(doors[i].exitY) << ",\n";
        file << TAB TAB ".tileset = " << static_cast<size_t>(doors[i].tileset) << ",\n";
        file << TAB "},\n";
    }

    file << "};\n";
}

void Parser::SaveTilesets(std::fstream& file, const std::string& symbolName)
{
    file << "\nconst u8* const " << symbolName << "[] = {\n";

    for (const std::string& tileset : tilesets)
        file << TAB << tileset << ",\n";

    file << "};\n";
}

void Parser::SaveCollisionTable(std::fstream& file, const std::string& symbolName)
{
    const CollisionTable& collisionTable = collisionTables[symbolName];

    file << "\nconst u8 " << symbolName << "[] = {\n";

    for (const std::string& clipdata : collisionTable)
        file << TAB << clipdata << ",\n";

    file << "};\n";
}

void Parser::SaveCollisionTableArray(std::fstream& file, const std::string& symbolName)
{
    file << "\nconst u8* const " << symbolName << "[] = {\n";

    for (const std::string& collisionTable : collisionTableArray)
        file << TAB << collisionTable << ",\n";

    file << "};\n";
}

void Parser::SavePhysics()
{
    const std::string fileName = Application::projectPath + R"(\src\player.c)";
    std::fstream file;
    std::string line;
    std::vector<std::string> lines;

    file.open(fileName);

    while (file)
    {
        std::getline(file, line);

        if (line.starts_with("static void PlayerInitPhysics"))
        {
            lines.push_back(line);
            std::getline(file, line);
            lines.push_back(line);
            std::getline(file, line);
            lines.push_back("    gPlayerPhysics.xAcceleration = " + std::to_string(physics.xAcceleration) + ';');
            std::getline(file, line);
            lines.push_back("    gPlayerPhysics.xVelocityCap = " + std::to_string(physics.xVelocityCap) + ';');
            std::getline(file, line);
            lines.push_back("    gPlayerPhysics.yVelocityCap = " + std::to_string(physics.yVelocityCap) + ';');
            std::getline(file, line);
            lines.push_back("    gPlayerPhysics.gravityUpwards = " + std::to_string(physics.gravityUpwards) + ';');
            std::getline(file, line);
            lines.push_back("    gPlayerPhysics.gravityDownwards = " + std::to_string(physics.gravityDownwards) + ';');
            std::getline(file, line);
            lines.push_back("    gPlayerPhysics.jumpingVelocity = " + std::to_string(physics.jumpingVelocity) + ';');
        }
        else
        {
            lines.push_back(line);
        }
    }

    std::fstream tempFile;
    tempFile.open("temp.txt", std::fstream::out | std::fstream::ate);
    for (size_t i = 0; i < lines.size() - 1; i++)
        tempFile << lines[i] << '\n';

    tempFile.close();
    file.close();

    (void)remove(fileName.c_str());
    (void)rename("temp.txt", fileName.c_str());
}

std::string Parser::ToHex(const size_t value)
{
    std::stringstream stream;

    stream << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << value;

    return stream.str();
}
