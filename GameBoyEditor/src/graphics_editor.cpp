#include "graphics_editor.hpp"

#include <ranges>

#include "parser.hpp"
#include "ui.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

void GraphicsEditor::Update()
{
    DrawGraphicsSelector();
    DrawPalette();
    if (m_SelectedGraphics != "<None>")
    {
        DrawCurrentTile();
        ImGui::SameLine();
        DrawGraphics();
    }
}

void GraphicsEditor::DrawGraphicsSelector()
{
    if (!ImGui::BeginCombo("Graphics", m_SelectedGraphics.c_str()))
        return;

    for (const std::string& s : Parser::graphics | std::ranges::views::keys)
    {
        if (ImGui::MenuItem(s.c_str()))
        {
            m_SelectedGraphics = s;
        }
    }

    ImGui::EndCombo();
}

void GraphicsEditor::DrawPalette()
{
    Ui::DrawPalette(m_Position, m_ColorPalette, 30.f, m_SelectedColor);
}

void GraphicsEditor::DrawGraphics()
{
    std::vector<uint8_t>& graphics = Parser::graphics[m_SelectedGraphics];

    Ui::CreateSubWindow("graphicsWindow", ImGuiChildFlags_ResizeX);

    size_t tileAmount = graphics.size() / 16;
    if (ImGui::Button("Add tile"))
    {
        for (size_t i = 0; i < 16; i++)
            graphics.push_back(0);
    }

    ImGui::BeginDisabled(tileAmount == 1);
    if (ImGui::Button("Delete tile"))
    {
        graphics.erase(graphics.begin() + static_cast<int64_t>(m_SelectedTile) * 16, graphics.begin() + static_cast<int64_t>(m_SelectedTile + 1) * 16);

        if (m_SelectedTile == tileAmount - 1)
            m_SelectedTile--;
    }
    ImGui::EndDisabled();

    const ImVec2 position = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY());

    Ui::DrawGraphics(position, graphics, m_ColorPalette, &m_SelectedTile);

    ImGui::EndChild();
}

void GraphicsEditor::DrawCurrentTile()
{
    std::vector<uint8_t>& graphics = Parser::graphics[m_SelectedGraphics];
    const size_t tileAmount = graphics.size() / 16;
    const size_t index = m_SelectedTile * 16;

    Ui::CreateSubWindow("tileWindow", ImGuiChildFlags_ResizeX);

    ImGui::SliderInt("Current tile", reinterpret_cast<int32_t*>(&m_SelectedTile), 0, static_cast<int32_t>(tileAmount - 1));
    ImGui::SliderInt("Pixel size", &m_PixelSize, 25, 40);

    const ImVec2 position = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY());

    Ui::DrawTile(position, graphics, index, m_ColorPalette, static_cast<float_t>(m_PixelSize));
    const size_t pixelIndex = Ui::DrawSelectSquare(position, ImVec2(8.f, 8.f), static_cast<float_t>(m_PixelSize));

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && pixelIndex != std::numeric_limits<size_t>::max())
    {
        const Color color = m_ColorPalette[m_SelectedColor];
        const size_t bitIndex = 7 - pixelIndex % 8;

        uint8_t& plane0 = graphics[index + pixelIndex / 8 * 2 + 0];
        uint8_t& plane1 = graphics[index + pixelIndex / 8 * 2 + 1];

        plane0 = static_cast<uint8_t>((plane0 & ~(1 << bitIndex)) | (color >> 0 & 1) << bitIndex);
        plane1 = static_cast<uint8_t>((plane1 & ~(1 << bitIndex)) | (color >> 1 & 1) << bitIndex);
    }

    ImGui::EndChild();
}
