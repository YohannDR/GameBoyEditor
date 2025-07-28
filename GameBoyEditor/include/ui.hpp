#pragma once
#include <vector>

#include "color.hpp"
#include "ui_window.hpp"

class Ui
{
public:
    static void Init();
    static void MainMenuBar();
    static void DrawWindows();
    static void OnProjectLoaded();

    static void DrawPalette(Palette& palette, float_t size, size_t* selectedColor);
    static void DrawTile(ImVec2 position, const std::vector<uint8_t>& graphics, size_t graphicsIndex, const Palette& palette, float_t size);
    static void DrawCross(ImVec2 position, float_t size);
    static void DrawGraphics(ImVec2 position, const std::vector<uint8_t>& graphics, const Palette& palette, size_t* selectedTile);
    static size_t DrawSelectSquare(ImVec2 position, ImVec2 size, float_t squareSize);

    static void CreateSubWindow(const char_t* name, ImGuiChildFlags flags, ImVec2 size = ImVec2(0, 0), uint32_t bgColor = IM_COL32(50, 50, 50, 255));

    template <typename T>
    static T* ShowWindow();

private:
    static inline std::vector<UiWindow*> m_Windows;
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
