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

    static void DrawPalette(ImVec2 position, Palette& palette, float_t size, size_t& selectedColor);
    static void DrawTile(ImVec2 position, const std::vector<uint8_t>& graphics, size_t graphicsIndex, const Palette& palette, float_t size);

    template <typename T>
    static void ShowWindow();

private:
    static inline std::vector<UiWindow*> m_Windows;
};

template <typename T>
void Ui::ShowWindow()
{
    for (UiWindow* const w : m_Windows)
    {
        if (dynamic_cast<T*>(w))
            w->hidden = false;
    }
}
