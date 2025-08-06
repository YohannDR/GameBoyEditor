#pragma once

#include "parser.hpp"
#include "render_target.hpp"
#include "ui_window.hpp"
#include "actions/edit_tilemap_action.hpp"

class RoomEditor : public UiWindow
{
public:
    explicit RoomEditor();

    void Update() override;
    void OnProjectLoaded() override;

private:
    enum class EditingMode : uint8_t
    {
        Tile,
        Object
    };
    
    void DrawOptions();
    void DrawRoomId();
    void DrawEditingMode();
    void DrawResize();
    void DrawTileset();
    void DrawRoom();

    void DrawSprites(ImVec2 position, bool_t inBounds, size_t cursorX, size_t cursorY);
    void DrawDoors(ImVec2 position, bool_t inBounds, size_t cursorX, size_t cursorY);

    void LoadRoom();
    void ResizeRoom();

    void ChangeEditingMode(EditingMode newMode);

    uint8_t m_Width = 0;
    uint8_t m_Height = 0;

    size_t m_RoomId = 0;
    size_t m_SelectedTile = 0;

    std::string m_SelectedGraphics = "<None>";

    EditingMode m_EditingMode = EditingMode::Tile;

    SpriteData* m_SelectedSprite = nullptr;
    SpriteData* m_HoveredSprite = nullptr;

    Door* m_SelectedDoor = nullptr;
    Door* m_HoveredDoor = nullptr;
    uint8_t m_SelectedDoorAnchorX = 0;
    uint8_t m_SelectedDoorAnchorY = 0;

    bool_t m_IsObjectEditPopupOpen = false;
    size_t m_BackupCursorX = 0;
    size_t m_BackupCursorY = 0;

    EditTilemapAction* m_EditTilemapAction = nullptr;

    RenderTarget m_GraphicsRenderTarget;
    RenderTarget m_TilemapRenderTarget;
};
