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
    void DrawOptions();
    void DrawRoomId();
    void DrawResize();
    void DrawTileset();
    void DrawRoom() const;

    void LoadRoom();
    void ResizeRoom() const;

    uint8_t m_Width = 0;
    uint8_t m_Height = 0;

    size_t m_RoomId = 0;
    size_t m_SelectedTile = 0;
};
