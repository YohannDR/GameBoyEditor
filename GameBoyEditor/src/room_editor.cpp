#include "room_editor.hpp"

#include <ranges>

#include "application.hpp"
#include "parser.hpp"
#include "ui.hpp"

void RoomEditor::Update()
{
    if (!Application::IsProjectLoaded())
        return;

    const float_t y = ImGui::GetCursorPosY();
    DrawOptions();
    DrawTileset();
    ImGui::SameLine();
    ImGui::SetCursorPosY(y);
    DrawRoom();
}

void RoomEditor::OnProjectLoaded()
{
    LoadRoom();
}

void RoomEditor::DrawOptions()
{
    Ui::CreateSubWindow("roomOptions", ImGuiChildFlags_ResizeY, ImVec2(4 * 8 * 16, 0));
    DrawRoomId();
    DrawResize();
    Ui::DrawPalette(Parser::rooms[m_RoomId].colorPalette, 30.f, nullptr);
    ImGui::EndChild();
}

void RoomEditor::DrawRoomId()
{
    const std::string nbr = std::to_string(m_RoomId);
    if (ImGui::BeginCombo("Room", nbr.c_str()))
    {
        for (size_t i = 0; i < Parser::rooms.size(); i++)
        {
            const std::string id = std::to_string(i);
            if (ImGui::Selectable(id.c_str(), i == m_RoomId))
            {
                LoadRoom();
                m_RoomId = i;
            }
        }

        ImGui::EndCombo();
    }
}

void RoomEditor::DrawResize()
{
    constexpr uint8_t maxWidth = 20 * 4;
    constexpr uint8_t maxHeight = 18 * 4;

    constexpr float_t size = 50;
    ImGui::SetNextItemWidth(size / 2);
    ImGui::DragScalar("Width", ImGuiDataType_U8, &m_Width, 1, nullptr, &maxWidth);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(size / 2);
    ImGui::DragScalar("Height", ImGuiDataType_U8, &m_Height, 1, nullptr, &maxHeight);

    if (ImGui::Button("Resize"))
        ResizeRoom();
}

void RoomEditor::DrawTileset()
{
    Ui::CreateSubWindow("roomTileset", ImGuiChildFlags_ResizeY, ImVec2(4 * 8 * 16, 0));

    if (ImGui::BeginCombo("Graphics", Parser::rooms[m_RoomId].graphics.c_str()))
    {
        for (const std::string& s : Parser::graphics | std::ranges::views::keys)
        {
            if (ImGui::MenuItem(s.c_str()))
                Parser::rooms[m_RoomId].graphics = s;
        }
        
        ImGui::EndCombo();
    }

    const std::vector<uint8_t>& graphics = Parser::graphics[Parser::rooms[m_RoomId].graphics];
    const ImVec2 position = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY());

    Ui::DrawGraphics(position, graphics, Parser::rooms[m_RoomId].colorPalette, &m_SelectedTile);
    ImGui::EndChild();
}

void RoomEditor::DrawRoom() const
{
    const std::vector<uint8_t>& graphics = Parser::graphics[Parser::rooms[m_RoomId].graphics];
    Tilemap& tilemap = Parser::tilemaps[Parser::rooms[m_RoomId].tilemap];
    const Palette palette = Parser::rooms[m_RoomId].colorPalette;

    const size_t height = tilemap.size();
    const size_t width = tilemap[0].size();

    Ui::CreateSubWindow("room", ImGuiChildFlags_ResizeX);

    ImGui::Text("Tilemap : %s", Parser::rooms[m_RoomId].tilemap.c_str());

    const ImVec2 position = ImVec2(ImGui::GetWindowPos().x - ImGui::GetScrollX(), ImGui::GetWindowPos().y + ImGui::GetCursorPosY() - ImGui::GetScrollY());

    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            const ImVec2 tilePosition = ImVec2(
                position.x + static_cast<float_t>(j) * 8 * PixelSize,
                position.y + static_cast<float_t>(i) * 8 * PixelSize
            );

            const uint8_t tileId = tilemap[i][j];
            if (tileId < graphics.size() / 16)
                Ui::DrawTile(tilePosition, graphics, static_cast<size_t>(tileId * 16), palette, PixelSize);
            else
                Ui::DrawCross(tilePosition, PixelSize);
        }
    }

    ImGui::Dummy(ImVec2(width * PixelSize * 8, height * PixelSize * 8));

    const size_t index = Ui::DrawSelectSquare(position, ImVec2(static_cast<float_t>(width), static_cast<float_t>(height)), PixelSize * 8);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && index != std::numeric_limits<size_t>::max())
    {
        const size_t x = index % width;
        const size_t y = index / width;

        tilemap[y][x] = static_cast<uint8_t>(m_SelectedTile);
    }

    DrawSprites(position);

    ImGui::EndChild();
}

void RoomEditor::DrawSprites(const ImVec2 position) const
{
    const std::vector<SpriteData>& spriteData = Parser::sprites[Parser::rooms[m_RoomId].spriteData];

    ImDrawList* const dl = ImGui::GetWindowDrawList();
    
    for (const SpriteData& sprite : spriteData)
    {
        const ImVec2 p1 = ImVec2(position.x + sprite.x * PixelSize * 8, position.y + sprite.y * PixelSize * 8);
        const ImVec2 p2 = ImVec2(p1.x + PixelSize * 8, p1.y + PixelSize * 8);

        dl->AddRect(p1, p2, IM_COL32(0x00, 0x00, 0xFF, 0xFF), 0, 0, 4);
    }
}

void RoomEditor::LoadRoom()
{
    const Tilemap& tilemap = Parser::tilemaps[Parser::rooms[m_RoomId].tilemap];

    m_Height = static_cast<uint8_t>(tilemap.size());
    m_Width = static_cast<uint8_t>(tilemap[0].size());
}

void RoomEditor::ResizeRoom() const
{
    Tilemap& tilemap = Parser::tilemaps[Parser::rooms[m_RoomId].tilemap];

    tilemap.resize(m_Height);

    for (std::vector<uint8_t>& v : tilemap)
        v.resize(m_Width);
}
