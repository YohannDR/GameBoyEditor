#pragma once

#include "color.hpp"
#include "render_target.hpp"
#include "ui_window.hpp"

class CollisionTableEditor : public UiWindow
{
public:
    explicit CollisionTableEditor();

    void Update() override;

private:
    void DrawTilesetSelector();
    void DrawCollisionInfo();
    void DrawCollisionTableSelector();
    void DrawTileset();

    static void DrawClipdataSelector(std::string& clipdata);

    std::string m_SelectedTileset = "<None>";
    std::string m_SelectedCollisionTable = "<None>";
    size_t m_SelectedTile = 0;

    RenderTarget m_TilesetRenderTarget;

    Palette m_ColorPalette = Palette{ White, LightGrey, DarkGrey, Black };
};
