#include "editors/graphics_editor.hpp"

#include <functional>
#include <ranges>

#include "parser.hpp"
#include "ui.hpp"
#include "actions/graphics_add_tile_action.hpp"
#include "actions/graphics_delete_tile_action.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

GraphicsEditor::GraphicsEditor()
{
    name = "Graphics editor";

    m_GraphicsRenderTarget.Create(16 * 8, 8);
    m_GraphicsRenderTarget.scale = 4;

    m_TileRenderTarget.Create(8, 8);
    m_TileRenderTarget.scale = 25;
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
            m_GraphicsRenderTarget.SetSize(16 * 8, static_cast<int32_t>((1 + tileMax / 16) * 8));
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

    ImGui::SliderFloat("Zoom", &m_GraphicsRenderTarget.scale, 4, 16);
    Ui::DrawGraphics(m_GraphicsRenderTarget, graphics, m_ColorPalette, &m_SelectedTile);

    ImGui::EndChild();
}

void GraphicsEditor::DrawCurrentTile()
{
    Graphics& graphics = Parser::graphics[m_SelectedGraphics];
    const size_t tileAmount = graphics.size() / 16;

    Ui::CreateSubWindow("tileWindow", ImGuiChildFlags_ResizeX);

    ImGui::SliderInt("Current tile", reinterpret_cast<int32_t*>(&m_SelectedTile), 0, static_cast<int32_t>(tileAmount - 1));
    ImGui::SliderFloat("Zoom", &m_TileRenderTarget.scale, 25, 40);

    m_SelectedTile = std::min(m_SelectedTile, tileAmount - 1);

    const ImVec2 position = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY());

    Ui::DrawTile(m_TileRenderTarget, graphics, m_SelectedTile, m_ColorPalette);
    const size_t pixelIndex = Ui::DrawSelectSquare(position, ImVec2(8.f, 8.f), m_TileRenderTarget.scale);

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && pixelIndex != std::numeric_limits<size_t>::max())
    {
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
        {
            PerformFill(pixelIndex);
        }
        else
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

void GraphicsEditor::PerformFill(const size_t pixelIndex)
{
    Graphics& graphics = Parser::graphics[m_SelectedGraphics];

    const size_t bitIndex = 7 - pixelIndex % 8;

    const size_t row = pixelIndex / 8;
    const uint8_t plane0 = graphics[m_SelectedTile * 16 + row * 2 + 0] >> bitIndex & 1;
    const uint8_t plane1 = graphics[m_SelectedTile * 16 + row * 2 + 1] >> bitIndex & 1;

    const Color oldColor = static_cast<Color>(plane0 | plane1 << 1);
    const Color newColor = m_ColorPalette[m_SelectedColor];

    if (oldColor == newColor)
        return;

    PlotPixelAction* action = new PlotPixelAction(&graphics);

    // https://www.geeksforgeeks.org/dsa/flood-fill-algorithm/
    const std::function<void(Graphics&, int32_t, int32_t)> dfs = [&dfs, this, oldColor, newColor, action]
        (Graphics& gfx, const int32_t x, const int32_t y)
    {
        if (x < 0 || x >= 8 || y < 0 || y >= 8)
            return;

        const size_t index = m_SelectedTile * 16 + static_cast<size_t>(y) * 2;

        const uint8_t p0 = gfx[index + 0];
        const uint8_t p1 = gfx[index + 1];

        const Color pixelColor = static_cast<Color>((p0 >> x & 1) | (p1 >> x & 1) << 1);
        if (pixelColor != oldColor)
            return;

        const uint8_t newPlane0 = static_cast<uint8_t>((p0 & ~(1 << x)) | (newColor >> 0 & 1) << x);
        const uint8_t newPlane1 = static_cast<uint8_t>((p1 & ~(1 << x)) | (newColor >> 1 & 1) << x);
        gfx[index + 0] = newPlane0;
        gfx[index + 1] = newPlane1;

        action->AddEdit(m_SelectedTile, y, p0, p1, newPlane0, newPlane1);

        dfs(gfx, x + 1, y);
        dfs(gfx, x - 1, y);
        dfs(gfx, x, y + 1);
        dfs(gfx, x, y - 1);
    };

    dfs(graphics, static_cast<uint8_t>(bitIndex), static_cast<uint8_t>(row));

    m_ActionQueue.Push(action);
}
