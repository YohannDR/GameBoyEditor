#pragma once

#include "ui_window.hpp"

class RoomEditor : public UiWindow
{
public:
    explicit RoomEditor() { name = "Room editor"; }

    void Update() override;

private:
    void DrawRoomId();
    void DrawTileset();
    void DrawRoom() const;

    size_t m_RoomId = 0;
    size_t m_SelectedTile = 0;
};
