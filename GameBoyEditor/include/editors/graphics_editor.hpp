#pragma once

#include "color.hpp"
#include "ui_window.hpp"

class GraphicsEditor : public UiWindow
{
public:
    explicit GraphicsEditor() { name = "Graphics editor"; }

    void Update() override;

private:
    void DrawGraphicsSelector();
    void DrawPalette();
    void DrawGraphics();
    void DrawCurrentTile();

    std::string m_SelectedGraphics = "<None>";
    Palette m_ColorPalette = { White, LightGrey, DarkGrey, Black };

    size_t m_SelectedColor = 0;
    size_t m_SelectedTile = 0;
    int32_t m_PixelSize = 25;
};
