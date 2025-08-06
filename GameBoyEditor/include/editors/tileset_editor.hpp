#pragma once

#include "door.hpp"
#include "ui_window.hpp"

class TilesetEditor : public UiWindow
{
public:
    explicit TilesetEditor() { name = "Tileset editor"; }

    void Update() override;

private:
    std::string m_SelectedGraphics = "<None>";
};
