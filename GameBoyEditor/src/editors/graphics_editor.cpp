#include "editors/graphics_editor.hpp"

#include <ranges>

#include "parser.hpp"
#include "ui.hpp"
#include "actions/graphics_add_tile_action.hpp"
#include "actions/graphics_delete_tile_action.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

GraphicsEditor::GraphicsEditor()
    : m_GraphicsRenderTarget(16 * 8, 8)
{
    name = "Graphics editor";
    m_GraphicsRenderTarget.scale = 4;
}

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

            const Graphics& gfx = Parser::graphics[s];
            const size_t tileMax = gfx.size() / 16;
            m_GraphicsRenderTarget.SetSize(16 * 8, (1 + tileMax / 16) * 8);
        }
    }

    ImGui::EndCombo();
}

void GraphicsEditor::DrawPalette()
{
    Ui::DrawPalette(m_ColorPalette, 30.f, &m_SelectedColor);
}

void GraphicsEditor::DrawGraphics()
{
    Graphics& graphics = Parser::graphics[m_SelectedGraphics];

    Ui::CreateSubWindow("graphicsWindow", ImGuiChildFlags_ResizeX);

    const size_t tileAmount = graphics.size() / 16;
    if (ImGui::Button("Add tile"))
    {
        for (size_t i = 0; i < 16; i++)
            graphics.push_back(0);

        m_ActionQueue.Push(new GraphicsAddTileAction(&graphics, tileAmount));
    }

    ImGui::BeginDisabled(tileAmount == 1);
    if (ImGui::Button("Delete tile"))
    {
        m_ActionQueue.Push(new GraphicsDeleteTileAction(&graphics, m_SelectedTile));

        graphics.erase(graphics.begin() + static_cast<int64_t>(m_SelectedTile) * 16, graphics.begin() + static_cast<int64_t>(m_SelectedTile + 1) * 16);

        if (m_SelectedTile == tileAmount - 1)
            m_SelectedTile--;
    }
    ImGui::EndDisabled();

    const ImVec2 position = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY());

    Ui::DrawGraphics(m_GraphicsRenderTarget, graphics, m_ColorPalette, &m_SelectedTile);

    ImGui::EndChild();
}

void GraphicsEditor::DrawCurrentTile()
{
    Graphics& graphics = Parser::graphics[m_SelectedGraphics];
    const size_t tileAmount = graphics.size() / 16;

    Ui::CreateSubWindow("tileWindow", ImGuiChildFlags_ResizeX);

    ImGui::SliderInt("Current tile", reinterpret_cast<int32_t*>(&m_SelectedTile), 0, static_cast<int32_t>(tileAmount - 1));
    ImGui::SliderInt("Pixel size", &m_PixelSize, 25, 40);

    m_SelectedTile = std::min(m_SelectedTile, tileAmount - 1);

    const ImVec2 position = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY());

    Ui::DrawTile(position, graphics, m_SelectedTile, m_ColorPalette, static_cast<float_t>(m_PixelSize));
    const size_t pixelIndex = Ui::DrawSelectSquare(position, ImVec2(8.f, 8.f), static_cast<float_t>(m_PixelSize));

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && pixelIndex != std::numeric_limits<size_t>::max())
    {
        if (m_PlotPixelAction == nullptr)
            m_PlotPixelAction = new PlotPixelAction(&graphics);

        const Color color = m_ColorPalette[m_SelectedColor];
        const size_t bitIndex = 7 - pixelIndex % 8;

        const size_t row = pixelIndex / 8;
        uint8_t& plane0 = graphics[m_SelectedTile * 16 + row * 2 + 0];
        uint8_t& plane1 = graphics[m_SelectedTile * 16 + row * 2 + 1];

        const uint8_t newPlane0 = static_cast<uint8_t>((plane0 & ~(1 << bitIndex)) | (color >> 0 & 1) << bitIndex);
        const uint8_t newPlane1 = static_cast<uint8_t>((plane1 & ~(1 << bitIndex)) | (color >> 1 & 1) << bitIndex);

        m_PlotPixelAction->AddEdit(static_cast<uint8_t>(m_SelectedTile), static_cast<uint8_t>(row), plane0, plane1, newPlane0, newPlane1);

        plane0 = newPlane0;
        plane1 = newPlane1;
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        if (m_PlotPixelAction)
        {
            m_ActionQueue.Push(m_PlotPixelAction);
            m_PlotPixelAction = nullptr;
        }
    }

    ImGui::EndChild();
}
