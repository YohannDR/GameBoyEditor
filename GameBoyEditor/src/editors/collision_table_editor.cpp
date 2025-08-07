#include "editors/collision_table_editor.hpp"

#include <ranges>

#include "parser.hpp"
#include "ui.hpp"

CollisionTableEditor::CollisionTableEditor()
{
    name = "Collision table editor";

    m_TilesetRenderTarget.Create(16 * 8, 8);
    m_TilesetRenderTarget.scale = 4;
}

void CollisionTableEditor::Update()
{
    DrawTilesetSelector();

    Ui::DrawPalette(m_ColorPalette, 30.f, nullptr);
    DrawCollisionInfo();

    if (m_SelectedTileset != "<None>")
    {
        ImGui::SameLine();
        DrawTileset();
    }
}

void CollisionTableEditor::DrawTilesetSelector()
{
    if (ImGui::BeginCombo("Tileset", m_SelectedTileset.c_str()))
    {
        for (const std::string& tileset : Parser::tilesets)
        {
            if (ImGui::MenuItem(tileset.c_str()))
            {
                m_SelectedTileset = tileset;

                const Graphics& gfx = Parser::graphics[tileset];
                const size_t tileMax = gfx.size() / 16;
                m_TilesetRenderTarget.SetSize(16 * 8, static_cast<int32_t>((1 + tileMax / 16) * 8));
            }
        }

        ImGui::EndCombo();
    }
}

void CollisionTableEditor::DrawCollisionInfo()
{
    Ui::CreateSubWindow("collisionInfo", ImGuiChildFlags_ResizeX);

    DrawCollisionTableSelector();

    if (m_SelectedCollisionTable != "<None>")
    {
        CollisionTable& colTable = Parser::collisionTables[m_SelectedCollisionTable];
        for (size_t i = 0; i < colTable.size(); i++)
        {
            ImGui::PushID(&i + i);
            if (m_SelectedTile == i)
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%02zu : ", i);
            else
                ImGui::Text("%02zu : ", i);
            ImGui::SameLine();
            DrawClipdataSelector(colTable[i]);

            ImGui::SameLine();
            ImGui::TextColored(m_SelectedTile == i ? ImVec4(1, 0, 0, 1) : ImVec4(0, 1, 0, 1), "X");
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                m_SelectedTile = i;
            ImGui::PopID();
        }
    }

    ImGui::EndChild();
}

void CollisionTableEditor::DrawCollisionTableSelector()
{
    if (ImGui::BeginCombo("Collision table", m_SelectedCollisionTable.c_str()))
    {
        for (const std::string& collisionTable : Parser::collisionTableArray)
        {
            if (ImGui::MenuItem(collisionTable.c_str()))
                m_SelectedCollisionTable = collisionTable;
        }

        ImGui::EndCombo();
    }
}

void CollisionTableEditor::DrawTileset()
{
    const Graphics& graphics = Parser::graphics[m_SelectedTileset];

    Ui::CreateSubWindow("graphicsWindow", ImGuiChildFlags_ResizeX);

    ImGui::SliderFloat("Zoom", &m_TilesetRenderTarget.scale, 4, 16);

    const size_t tileCount = graphics.size() / 16;
    CollisionTable& collisionTable = Parser::collisionTables[m_SelectedCollisionTable];
    const size_t previousSize = collisionTable.size();

    ImGui::BeginDisabled(m_SelectedCollisionTable == "<None>" || tileCount == previousSize);
    if (ImGui::Button("Resize table to fit graphics"))
    {
        collisionTable.resize(tileCount);

        for (size_t i = previousSize; i < tileCount; i++)
            collisionTable[i] = "CLIPDATA_AIR";
    }
    ImGui::EndDisabled();

    Ui::DrawGraphics(m_TilesetRenderTarget, graphics, m_ColorPalette, &m_SelectedTile);

    ImGui::EndChild();
}

void CollisionTableEditor::DrawClipdataSelector(std::string& clipdata)
{
    if (ImGui::BeginCombo("##clipdata", clipdata.c_str()))
    {
        for (const std::string& c : Parser::clipdataNames)
        {
            if (ImGui::MenuItem(c.c_str()))
                clipdata = c;
        }

        ImGui::EndCombo();
    }
}
