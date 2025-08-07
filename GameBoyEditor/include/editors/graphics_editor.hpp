#pragma once

#include "color.hpp"
#include "render_target.hpp"
#include "ui_window.hpp"
#include "actions/plot_pixel_action.hpp"

class GraphicsEditor : public UiWindow
{
public:
    explicit GraphicsEditor();

    void Update() override;

private:
    void DrawGraphicsSelector();
    void DrawPalette();
    void DrawGraphics();
    void DrawCurrentTile();

    void PerformFill(size_t pixelIndex);

    std::string m_SelectedGraphics = "<None>";
    Palette m_ColorPalette = { White, LightGrey, DarkGrey, Black };

    size_t m_SelectedColor = 0;
    size_t m_SelectedTile = 0;

    PlotPixelAction* m_PlotPixelAction = nullptr;

    RenderTarget m_GraphicsRenderTarget;
    RenderTarget m_TileRenderTarget;
};
