#include "room_editor.hpp"

#include "application.hpp"
#include "parser.hpp"
#include "ui.hpp"

void RoomEditor::Update()
{
    if (!Application::IsProjectLoaded())
        return;

    DrawRoomId();
    DrawResize();
    DrawTileset();
    ImGui::SameLine();
    DrawRoom();
}

void RoomEditor::OnProjectLoaded()
{
    LoadRoom();
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
    constexpr float_t size = 50;
    ImGui::SetNextItemWidth(size / 2);
    ImGui::InputScalar("Width", ImGuiDataType_U8, &m_Width);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(size / 2);
    ImGui::InputScalar("Height", ImGuiDataType_U8, &m_Height);

    if (ImGui::Button("Resize"))
        ResizeRoom();
}

void RoomEditor::DrawTileset()
{
    const std::vector<uint8_t>& graphics = Parser::graphics[Parser::rooms[m_RoomId].graphics];

    Ui::CreateSubWindow("roomTileset", ImGuiChildFlags_ResizeX);

    Ui::DrawGraphics(ImGui::GetWindowPos(), graphics, Parser::rooms[m_RoomId].colorPalette, &m_SelectedTile);
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

    const ImVec2 position = ImGui::GetWindowPos();

    constexpr float_t pixelSize = 4;
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            const ImVec2 tilePosition = ImVec2(
                position.x + static_cast<float_t>(j) * 8 * pixelSize,
                position.y + static_cast<float_t>(i) * 8 * pixelSize
            );

            const uint8_t tileId = tilemap[i][j];
            if (tileId < graphics.size() / 16)
                Ui::DrawTile(tilePosition, graphics, static_cast<size_t>(tileId * 16), palette, pixelSize);
            else
                Ui::DrawCross(tilePosition, pixelSize);
        }
    }

    const size_t index = Ui::DrawSelectSquare(position, ImVec2(static_cast<float_t>(width), static_cast<float_t>(height)), pixelSize * 8);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && index != std::numeric_limits<size_t>::max())
    {
        const size_t x = index % width;
        const size_t y = index / width;

        tilemap[y][x] = static_cast<uint8_t>(m_SelectedTile);
    }

    ImGui::EndChild();
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
