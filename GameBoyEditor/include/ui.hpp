#pragma once
#include <vector>

#include "color.hpp"
#include "parser.hpp"
#include "render_target.hpp"
#include "shader.hpp"
#include "ui_window.hpp"

class Ui
{
public:
    static void Init();
    static void MainMenuBar();
    static void DrawWindows();
    static void OnProjectLoaded();

    static void DrawTile(const RenderTarget& renderTarget, const Graphics& graphics, size_t graphicsIndex, const Palette& palette, bool_t xFlip = false, bool_t yFlip = false);
    static void DrawGraphics(const RenderTarget& renderTarget, const Graphics& graphics, const Palette& palette, size_t* selectedTile);
    static void DrawTilemap(const RenderTarget& renderTarget, const Graphics& graphics, const Tilemap& tilemap, const Palette& palette);

    static void DrawPalette(Palette& palette, float_t size, size_t* selectedColor);
    static void DrawCross(ImVec2 position, float_t size);
    static size_t DrawSelectSquare(ImVec2 position, ImVec2 size, float_t squareSize);

    static void CreateSubWindow(const char_t* name, ImGuiChildFlags flags, ImVec2 size = ImVec2(0, 0), uint32_t bgColor = IM_COL32(50, 50, 50, 255));

    template <typename T>
    static T* ShowWindow();

    _NODISCARD static ImVec2 GetPosition();

private:
    static void SetupWindow();
    static void SetupRenderer();


    static inline std::vector<UiWindow*> m_Windows;

    static inline Shader m_GraphicsShader;
    static inline Shader m_TilemapShader;

    static inline Texture m_GraphicsTexture;
    static inline Texture m_TilemapTexture;
};

template <typename T>
T* Ui::ShowWindow()
{
    for (UiWindow* const w : m_Windows)
    {
        T* const tW = dynamic_cast<T*>(w);
        if (tW)
        {
            w->open = true;
            return tW;
        }
    }

    return nullptr;
}
