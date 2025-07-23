#include "graphics_editor.hpp"

#include <ranges>

#include "parser.hpp"
#include "ui.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

void GraphicsEditor::Update()
{
    ImGui::ShowDemoWindow();

    DrawGraphicsSelector();
    DrawPalette();
    if (m_SelectedGraphics != "<None>")
    {
        DrawCurrentTile();
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
    const std::vector<uint8_t>& graphics = Parser::graphics[m_SelectedGraphics];

    const size_t tileAmount = graphics.size() / 16;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));
    ImGui::BeginChild("graphicsWindow", ImVec2(0.f, 250.f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX, ImGuiWindowFlags_NoMove);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    const ImVec2 basePos = ImGui::GetWindowPos();

    for (size_t i = 0; i < tileAmount; i++)
    {
        constexpr float_t pixelSize = 4;
        const ImVec2 tilePosition = ImVec2(
            basePos.x + static_cast<float_t>(i % 16) * 8 * pixelSize,
            basePos.y + static_cast<float_t>(i / 16) * 8 * pixelSize
        );

        Ui::DrawTile(tilePosition, graphics, i * 16, m_ColorPalette, pixelSize);
    }

    ImGui::EndChild();
}

void GraphicsEditor::DrawCurrentTile()
{
    const std::vector<uint8_t>& graphics = Parser::graphics[m_SelectedGraphics];
    const size_t tileAmount = graphics.size() / 16;
    const size_t index = m_SelectedTile * 16;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(50, 50, 50, 255));
    ImGui::BeginChild("tileWindow", ImVec2(0.f, 250.f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX, ImGuiWindowFlags_NoMove);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::SliderInt("Current tile", reinterpret_cast<int32_t*>(&m_SelectedTile), 0, static_cast<int32_t>(tileAmount - 1));
    ImGui::SliderInt("Pixel size", &m_PixelSize, 25, 40);

    const ImVec2 position = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY());

    Ui::DrawTile(position, graphics, index, m_ColorPalette, static_cast<float_t>(m_PixelSize));

    ImGui::EndChild();
}
