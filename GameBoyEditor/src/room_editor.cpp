#include "room_editor.hpp"

#include "application.hpp"
#include "parser.hpp"
#include "ui.hpp"

void RoomEditor::Update()
{
    if (!Application::IsProjectLoaded())
        return;

    DrawRoomId();
    DrawTileset();
    ImGui::SameLine();
    DrawRoom();
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
                m_RoomId = i;
        }

        ImGui::EndCombo();
    }
}

void RoomEditor::DrawTileset()
{
    const std::vector<uint8_t>& graphics = Parser::graphics[Parser::rooms[m_RoomId].graphics];

    Ui::CreateSubWindow("roomTileset", ImGuiChildFlags_ResizeX);

    Ui::DrawGraphics(ImGui::GetWindowPos(), graphics, Parser::rooms[m_RoomId].colorPalette, &m_SelectedTile);
    ImGui::EndChild();
}

void RoomEditor::DrawRoom()
{
    const std::vector<uint8_t>& graphics = Parser::graphics[Parser::rooms[m_RoomId].graphics];
    Tilemap& tilemap = Parser::tilemaps[Parser::rooms[m_RoomId].tilemap];
    const Palette palette = Parser::rooms[m_RoomId].colorPalette;

    Ui::CreateSubWindow("room", ImGuiChildFlags_ResizeX);

    const ImVec2 position = ImGui::GetWindowPos();

    constexpr float_t pixelSize = 4;
    for (size_t i = 0; i < tilemap.height; i++)
    {
        for (size_t j = 0; j < tilemap.width; j++)
        {
            const ImVec2 tilePosition = ImVec2(
                position.x + static_cast<float_t>(j) * 8 * pixelSize,
                position.y + static_cast<float_t>(i) * 8 * pixelSize
            );

            Ui::DrawTile(tilePosition, graphics, static_cast<size_t>(tilemap.data[i * tilemap.width + j] * 16), palette, pixelSize);
        }
    }

    const size_t index = Ui::DrawSelectSquare(position, ImVec2(tilemap.width, tilemap.height), pixelSize * 8);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && index != std::numeric_limits<size_t>::max())
    {
        const size_t x = index % tilemap.width;
        const size_t y = index / tilemap.width;

        tilemap.data[y * tilemap.width + x] = static_cast<uint8_t>(m_SelectedTile);
    }

    ImGui::EndChild();
}
