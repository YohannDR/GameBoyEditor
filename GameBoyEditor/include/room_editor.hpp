#pragma once

#include "parser.hpp"
#include "ui_window.hpp"

class RoomEditor : public UiWindow
{
public:
    explicit RoomEditor() { name = "Room editor"; }

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

    void LoadRoom();
    void ResizeRoom() const;

    uint8_t m_Width = 0;
    uint8_t m_Height = 0;

    size_t m_RoomId = 0;
    size_t m_SelectedTile = 0;

    EditingMode m_EditingMode = EditingMode::Tile;

    SpriteData* m_SelectedSprite = nullptr;

    static constexpr float_t PixelSize = 4;
};
