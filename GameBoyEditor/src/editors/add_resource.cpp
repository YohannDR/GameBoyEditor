#include "editors/add_resource.hpp"

#include <fstream>
#include <ranges>

#include "application.hpp"
#include "imgui/imgui_stdlib.h"
#include "magic_enum/magic_enum.hpp"

void AddResource::Setup(const SymbolType type)
{
    m_Type = type;
    m_FileName = "";
    m_SymbolName = "";

    m_RoomCollisionTable = 0;
}

void AddResource::Update()
{
    ImGui::Text("Adding %*s", static_cast<int32_t>(magic_enum::enum_name(m_Type).size()), magic_enum::enum_name(m_Type).data());

    if (m_Type == SymbolType::RoomData)
        RoomFields();
    else if (m_Type == SymbolType::CollisionTable)
        CollisionTableFields();
    else
        NormalSymbolFields();
}

void AddResource::NormalSymbolFields()
{
    ImGui::InputText("File name", &m_FileName);
    ImGui::SetItemTooltip("In which file the new resource should be put");
    ImGui::InputText("Symbol name", &m_SymbolName);
    ImGui::SetItemTooltip("The name of the resource, should start with an underscore 's'");

    m_FullFilePath = Application::projectPath + '\\' + m_FileName;

    const bool_t fileExists = std::filesystem::exists(m_FullFilePath) && std::filesystem::is_regular_file(m_FullFilePath);

    const bool_t symbolAlreadyExists = std::ranges::contains(Parser::existingSymbols, m_SymbolName);
    const bool_t symbolPrefix = m_SymbolName[0] == 's';
    const bool_t nameValid = !m_SymbolName.empty() && symbolPrefix && !symbolAlreadyExists;

    if (!fileExists)
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "File is invalid");

    if (symbolAlreadyExists)
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "A symbol with this name already exists");

    if (!symbolPrefix)
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Symbol name should start with an underscore 's'");

    ImGui::BeginDisabled(!fileExists || !nameValid);
    if (ImGui::Button("Add"))
    {
        CreateResource();
        open = false;
    }
    ImGui::EndDisabled();
}

void AddResource::RoomFields()
{
    ImGui::SameLine();
    ImGui::Text("%d", static_cast<int32_t>(Parser::rooms.size()));

    if (ImGui::BeginCombo("Collision table", Parser::collisionTableArray[m_RoomCollisionTable].c_str()))
    {
        for (size_t i = 0; i < Parser::collisionTableArray.size(); i++)
        {
            if (ImGui::MenuItem(Parser::collisionTableArray[i].c_str()))
                m_RoomCollisionTable = i;
        }

        ImGui::EndCombo();
    }

    if (ImGui::Button("Add"))
    {
        CreateRoom();
        open = false;
    }
}

void AddResource::CollisionTableFields()
{
    ImGui::Text("sCollisionTable_");
    ImGui::SameLine();
    ImGui::InputText("Symbol name", &m_SymbolName);
    ImGui::SetItemTooltip("The name of the resource, will be prefixed with \"sCollisionTable_\", you don't have to input it manually");

    if (ImGui::Button("Add"))
    {
        m_SymbolName = "sCollisionTable_" + m_SymbolName;
        CreateCollisionTable();
        open = false;
    }
}

void AddResource::CreateResource() const
{
    switch (m_Type)
    {
        case SymbolType::Graphics: CreateGraphics(); break;
        case SymbolType::Animation: CreateAnimation(); break;
        case SymbolType::CollisionTable: CreateCollisionTable(); break;

        case SymbolType::RoomData:
        case SymbolType::Tilemap:
        case SymbolType::DoorData:
        case SymbolType::SpriteData:
        case SymbolType::Doors:
        case SymbolType::Tilesets:
        case SymbolType::CollisionTableArray:
            return;
    }

    Parser::RegisterSymbol(m_FullFilePath, m_SymbolName, m_Type);
}

void AddResource::CreateGraphics() const
{
    const std::vector<uint8_t> dummyGraphics(16);
    Parser::graphics[m_SymbolName] = dummyGraphics;
}

void AddResource::CreateAnimation() const
{
    Animation dummyAnimation(1);
    dummyAnimation[0].oam.emplace_back(0, 0, 0, 0);
    dummyAnimation[0].duration = 60;
    Parser::animations[m_SymbolName] = dummyAnimation;
}

void AddResource::CreateCollisionTable() const
{
    const std::string sourceFile = Application::projectPath + R"(\src\data\collision_tables.c)";

    Parser::collisionTables[m_SymbolName] = {};
    Parser::collisionTables[m_SymbolName].emplace_back("CLIPDATA_AIR");

    Parser::collisionTableArray.push_back(m_SymbolName);
    Parser::RegisterSymbol(sourceFile, m_SymbolName, SymbolType::CollisionTable);

    RegenerateCollisionTableIncludeFile();
}

void AddResource::CreateRoom() const
{
    const std::string roomIndex = std::to_string(Parser::rooms.size());
    const std::string sourceFile = Application::projectPath + R"(\src\data\rooms\room)" + roomIndex + ".c";

    CreateRoomHeaderFile();

    const std::string tilemapName = std::string("sRoom") + roomIndex + "_Tilemap";
    Tilemap dummyTilemap;
    dummyTilemap.emplace_back(1);
    Parser::tilemaps[tilemapName] = dummyTilemap;
    Parser::RegisterSymbol(sourceFile, tilemapName, SymbolType::Tilemap);

    const std::string spriteDataName = std::string("sRoom") + roomIndex + "_SpriteData";
    Parser::sprites[spriteDataName] = {};
    Parser::RegisterSymbol(sourceFile, spriteDataName, SymbolType::SpriteData);

    const std::string doorDataName = std::string("sRoom") + roomIndex + "_DoorData";
    Parser::roomsDoorData[doorDataName] = {};
    Parser::RegisterSymbol(sourceFile, doorDataName, SymbolType::DoorData);

    constexpr Palette dummyPalette = { White, LightGrey, DarkGrey, Black };
    Parser::rooms.emplace_back(tilemapName, dummyPalette, spriteDataName, doorDataName, m_RoomCollisionTable);

    RegenerateRoomIncludeFile();
}

void AddResource::RegenerateRoomIncludeFile()
{
    const std::string fileName = Application::projectPath + R"(\include\data\rooms.h)";

    std::ofstream file;
    file.open(fileName);

    file << "#ifndef ROOMS_H\n#define ROOMS_H\n\n";

    for (size_t i = 0; i < Parser::rooms.size(); i++)
        file << "#include \"data/rooms/room" << i << ".h\"\n";

    file << "\n#endif /* ROOMS_H */\n";

    file.close();
}

void AddResource::CreateRoomHeaderFile()
{
    const std::string roomIndex = std::to_string(Parser::rooms.size());
    const std::string headerFile = Application::projectPath + R"(\include\data\rooms\room)" + roomIndex + ".h";

    std::ofstream file;
    file.open(headerFile);

    file << "#ifndef ROOM_" << roomIndex << "_H\n#define ROOM_" << roomIndex << "_H\n\n#include \"room.h\"\n\n";
    file << "extern const u8 sRoom" << roomIndex << "_Tilemap[];\n";
    file << "extern const struct RoomSprite sRoom" << roomIndex << "_SpriteData[];\n";
    file << "extern const u8 sRoom" << roomIndex << "_DoorData[];\n";

    file << "\n#endif /* ROOM_" << roomIndex << "_H */\n";
}

void AddResource::RegenerateCollisionTableIncludeFile()
{
    const std::string fileName = Application::projectPath + R"(\include\data\collision_tables.h)";

    std::ofstream file;
    file.open(fileName);

    file << "#ifndef COLLISION_TABLES_H\n#define COLLISION_TABLES_H\n\n#include \"types.h\"\n\n";

    for (const std::string& collisionTable : Parser::collisionTableArray)
        file << "extern const u8 " << collisionTable << "[];\n";

    file << "extern const u8* const sCollisionTables[];\n";

    file << "\n#endif /* COLLISION_TABLES_H */\n";

    file.close();
}
