#include "editors/room_editor.hpp"

#include <functional>
#include <iostream>
#include <ranges>

#include "application.hpp"
#include "parser.hpp"
#include "ui.hpp"
#include "editors/edit_door_window.hpp"
#include "editors/edit_sprite_window.hpp"

RoomEditor::RoomEditor()
{
    name = "Room editor";
    canBeClosed = false;

    m_GraphicsRenderTarget.Create(16 * 8, 8);
    m_GraphicsRenderTarget.scale = 4;

    m_TilemapRenderTarget.Create(8, 8);
    m_TilemapRenderTarget.scale = 4;
}

void RoomEditor::Update()
{
    if (!Application::IsProjectLoaded())
        return;

    const float_t y = ImGui::GetCursorPosY();
    DrawOptions();
    DrawTileset();

    if (m_SelectedGraphics == "<None>")
        return;

    ImGui::SameLine();
    ImGui::SetCursorPosY(y);
    DrawRoom();

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
        ChangeEditingMode(m_EditingMode == EditingMode::Tile ? EditingMode::Object : EditingMode::Tile);
}

void RoomEditor::OnProjectLoaded()
{
    LoadRoom();
}

void RoomEditor::DrawOptions()
{
    Ui::CreateSubWindow("roomOptions", ImGuiChildFlags_ResizeY, ImVec2(4 * 8 * 16, 0));
    DrawRoomId();
    DrawEditingMode();
    
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
                m_RoomId = i;
                LoadRoom();
            }
        }

        ImGui::EndCombo();
    }
}

void RoomEditor::DrawEditingMode()
{
    bool_t changed = ImGui::RadioButton("Tile mode", reinterpret_cast<int*>(&m_EditingMode), 0);
    ImGui::SameLine();
    changed |= ImGui::RadioButton("Object mode", reinterpret_cast<int*>(&m_EditingMode), 1);

    if (changed)
        ChangeEditingMode(m_EditingMode);
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

    if (ImGui::BeginCombo("Graphics", m_SelectedGraphics.c_str()))
    {
        for (const std::string& s : Parser::tilesets)
        {
            if (ImGui::MenuItem(s.c_str()))
            {
                m_SelectedGraphics = s;

                const Graphics& gfx = Parser::graphics[s];
                const size_t tileMax = gfx.size() / 16;
                m_GraphicsRenderTarget.SetSize(16 * 8, static_cast<int32_t>((1 + tileMax / 16) * 8));
            }
        }
        
        ImGui::EndCombo();
    }

    if (m_SelectedGraphics != "<None>")
    {
        const Graphics& graphics = Parser::graphics[m_SelectedGraphics];

        ImGui::SliderFloat("Zoom", &m_GraphicsRenderTarget.scale, 4, 20);
        Ui::DrawGraphics(m_GraphicsRenderTarget, graphics, Parser::rooms[m_RoomId].colorPalette, &m_SelectedTile);
    }

    ImGui::EndChild();
}

void RoomEditor::DrawRoom()
{
    const Graphics& graphics = Parser::graphics[m_SelectedGraphics];
    Tilemap& tilemap = Parser::tilemaps[Parser::rooms[m_RoomId].tilemap];
    const Palette palette = Parser::rooms[m_RoomId].colorPalette;

    const size_t height = tilemap.size();
    const size_t width = tilemap[0].size();

    Ui::CreateSubWindow("room", ImGuiChildFlags_ResizeX);

    ImGui::Text("Tilemap : %s", Parser::rooms[m_RoomId].tilemap.c_str());
    ImGui::SliderFloat("Zoom", &m_TilemapRenderTarget.scale, 4, 20);

    const ImVec2 position = Ui::GetPosition();

    Ui::DrawTilemap(m_TilemapRenderTarget, graphics, tilemap, palette);

    const size_t index = Ui::DrawSelectSquare(position, ImVec2(static_cast<float_t>(width), static_cast<float_t>(height)), m_TilemapRenderTarget.scale * 8);
    const bool_t inBounds = index != std::numeric_limits<size_t>::max();

    const size_t x = index % width;
    const size_t y = index / width;

    if (m_EditingMode == EditingMode::Tile && ImGui::IsMouseDown(ImGuiMouseButton_Left) && inBounds && ImGui::IsItemHovered())
    {
        if (!m_EditTilemapAction)
            m_EditTilemapAction = new EditTilemapAction(&tilemap);

        m_EditTilemapAction->AddEdit(static_cast<uint8_t>(x), static_cast<uint8_t>(y), tilemap[y][x], static_cast<uint8_t>(m_SelectedTile));
        tilemap[y][x] = static_cast<uint8_t>(m_SelectedTile);
    }

    if (m_EditingMode == EditingMode::Tile && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        if (m_EditTilemapAction)
        {
            m_ActionQueue.Push(m_EditTilemapAction);
            m_EditTilemapAction = nullptr;
        }
    }

    DrawSprites(position, inBounds, x, y);
    DrawDoors(position, inBounds, x, y);

    ImGui::EndChild();
}

void RoomEditor::DrawSprites(const ImVec2 position, const bool_t inBounds, const size_t cursorX, const size_t cursorY)
{
    std::vector<SpriteData>& spriteData = Parser::sprites[Parser::rooms[m_RoomId].spriteData];

    ImDrawList* const dl = ImGui::GetWindowDrawList();

    if (!m_IsObjectEditPopupOpen)
        m_HoveredSprite = nullptr;

    for (SpriteData& sprite : spriteData)
    {
        if (m_EditingMode == EditingMode::Object)
        {
            if (m_SelectedSprite == &sprite)
            {
                if (inBounds)
                {
                    m_SelectedSprite->x = static_cast<uint8_t>(cursorX);
                    m_SelectedSprite->y = static_cast<uint8_t>(cursorY + 1);
                }

                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    m_SelectedSprite = nullptr;
            }

            if (inBounds && cursorX == sprite.x && cursorY + 1 == sprite.y)
            {
                if (!m_IsObjectEditPopupOpen)
                    m_HoveredSprite = &sprite;

                if (m_SelectedSprite == nullptr && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    m_SelectedSprite = &sprite;
            }
        }
        else
        {
            m_SelectedSprite = nullptr;
        }

        const ImVec2 p1 = ImVec2(
            position.x + static_cast<float_t>(sprite.x) * m_TilemapRenderTarget.scale * 8,
            position.y + static_cast<float_t>(sprite.y - 1) * m_TilemapRenderTarget.scale * 8
        );
        const ImVec2 p2 = ImVec2(p1.x + m_TilemapRenderTarget.scale * 8, p1.y + m_TilemapRenderTarget.scale * 8);

        dl->AddRect(p1, p2, IM_COL32(0x00, 0xFF, 0x00, 0xFF), 0, 0, 4);
    }

    if (inBounds && m_EditingMode == EditingMode::Object)
    {
        if (ImGui::BeginPopupContextItem("objectEditPopup"))
        {
            if (!m_IsObjectEditPopupOpen)
            {
                m_BackupCursorX = cursorX;
                m_BackupCursorY = cursorY + 1;
                m_IsObjectEditPopupOpen = true;
            }

            ImGui::Text("Object menu");

            ImGui::SeparatorText("Sprite");

            ImGui::BeginDisabled(spriteData.size() == 20);
            if (ImGui::Button("Add sprite"))
            {
                spriteData.emplace_back(m_BackupCursorX, m_BackupCursorY, "STYPE_NONE", 0);

                ImGui::CloseCurrentPopup();
                m_IsObjectEditPopupOpen = false;
            }
            ImGui::EndDisabled();

            ImGui::BeginDisabled(m_HoveredSprite == nullptr);
            if (ImGui::Button("Edit sprite"))
            {
                Ui::ShowWindow<EditSpriteWindow>()->Setup(m_HoveredSprite);

                ImGui::CloseCurrentPopup();
                m_IsObjectEditPopupOpen = false;
            }

            if (ImGui::Button("Remove sprite"))
            {
                std::erase(spriteData, *m_HoveredSprite);
                m_HoveredSprite = nullptr;

                ImGui::CloseCurrentPopup();
                m_IsObjectEditPopupOpen = false;
            }
            ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        if (!ImGui::IsPopupOpen("objectEditPopup"))
            m_IsObjectEditPopupOpen = false;
    }
}

void RoomEditor::DrawDoors(const ImVec2 position, const bool_t inBounds, const size_t cursorX, const size_t cursorY)
{
    DoorData& doorData = Parser::roomsDoorData[Parser::rooms[m_RoomId].doorData];

    ImDrawList* const dl = ImGui::GetWindowDrawList();

    if (!m_IsObjectEditPopupOpen)
        m_HoveredDoor = nullptr;

    for (const uint8_t doorId : doorData)
    {
        Door& door = Parser::doors[doorId];

        if (m_EditingMode == EditingMode::Object)
        {
            if (m_SelectedDoor == &door)
            {
                if (inBounds)
                {
                    m_SelectedDoor->x = static_cast<uint8_t>(cursorX - m_SelectedDoorAnchorX);
                    m_SelectedDoor->y = static_cast<uint8_t>(cursorY - m_SelectedDoorAnchorY);
                }

                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    m_SelectedDoor = nullptr;
            }

            if (inBounds &&
                cursorX >= door.x && cursorX < static_cast<uint8_t>(door.x + door.width) &&
                cursorY >= door.y && cursorY < static_cast<uint8_t>(door.y + door.height))
            {
                if (!m_IsObjectEditPopupOpen)
                    m_HoveredDoor = &door;

                if (m_SelectedDoor == nullptr && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    m_SelectedDoor = &door;
                    m_SelectedDoorAnchorX = cursorX - door.x;
                    m_SelectedDoorAnchorY = cursorY - door.y;
                }
            }
        }
        else
        {
            m_SelectedDoor = nullptr;
        }

        const ImVec2 p1 = ImVec2(
            position.x + static_cast<float_t>(door.x) * m_TilemapRenderTarget.scale * 8,
            position.y + static_cast<float_t>(door.y) * m_TilemapRenderTarget.scale * 8
        );

        const ImVec2 p2 = ImVec2(
            p1.x + m_TilemapRenderTarget.scale * 8 * static_cast<float_t>(door.width),
            p1.y + m_TilemapRenderTarget.scale * 8 * static_cast<float_t>(door.height)
        );

        dl->AddRect(p1, p2, IM_COL32(0x00, 0x00, 0xFF, 0xFF), 0, 0, 4);
    }

    if (inBounds && m_EditingMode == EditingMode::Object)
    {
        if (ImGui::BeginPopupContextItem("objectEditPopup"))
        {
            ImGui::SeparatorText("Door");

            if (!m_IsObjectEditPopupOpen)
            {
                m_BackupCursorX = cursorX;
                m_BackupCursorY = cursorY + 1;
                m_IsObjectEditPopupOpen = true;
            }

            ImGui::BeginDisabled(doorData.size() == 4 || Parser::doors.size() == 255);
            if (ImGui::Button("Add door"))
            {
                doorData.push_back(static_cast<uint8_t>(Parser::doors.size()));
                Parser::doors.emplace_back(m_BackupCursorX, m_BackupCursorY, m_RoomId, 1, 1, 0, 0, 0, 0xFF);

                ImGui::CloseCurrentPopup();
                m_IsObjectEditPopupOpen = false;
            }
            ImGui::EndDisabled();

            ImGui::BeginDisabled(m_HoveredDoor == nullptr);
            if (ImGui::Button("Edit door"))
            {
                Ui::ShowWindow<EditDoorWindow>()->Setup(m_HoveredDoor);

                ImGui::CloseCurrentPopup();
                m_IsObjectEditPopupOpen = false;
            }

            if (ImGui::Button("Remove door"))
            {
                std::erase(doorData, std::ranges::find(Parser::doors, *m_HoveredDoor) - Parser::doors.begin());
                Parser::DeleteDoor(*m_HoveredDoor);

                m_HoveredDoor = nullptr;

                ImGui::CloseCurrentPopup();
                m_IsObjectEditPopupOpen = false;
            }
            ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        if (!ImGui::IsPopupOpen("objectEditPopup"))
            m_IsObjectEditPopupOpen = false;
    }
}

void RoomEditor::LoadRoom()
{
    const Tilemap& tilemap = Parser::tilemaps[Parser::rooms[m_RoomId].tilemap];

    m_Height = static_cast<uint8_t>(tilemap.size());
    m_Width = static_cast<uint8_t>(tilemap[0].size());
    m_TilemapRenderTarget.SetSize(m_Width * 8, m_Height * 8);
}

void RoomEditor::ResizeRoom()
{
    Tilemap& tilemap = Parser::tilemaps[Parser::rooms[m_RoomId].tilemap];

    tilemap.resize(m_Height);

    for (std::vector<uint8_t>& v : tilemap)
        v.resize(m_Width);

    m_TilemapRenderTarget.SetSize(m_Width * 8, m_Height * 8);
}

void RoomEditor::ChangeEditingMode(const EditingMode newMode)
{
    m_EditingMode = newMode;

    if (m_EditingMode == EditingMode::Object)
    {
        if (m_EditTilemapAction)
        {
            m_ActionQueue.Push(m_EditTilemapAction);
            m_EditTilemapAction = nullptr;
        }
    }
}
