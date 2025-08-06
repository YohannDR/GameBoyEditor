#include "editors/tileset_editor.hpp"

#include <ranges>

#include "parser.hpp"

void TilesetEditor::Update()
{
    ImGui::BeginDisabled(m_SelectedGraphics == "<None>");
    if (ImGui::Button("Add"))
    {
        Parser::tilesets.push_back(m_SelectedGraphics);
        m_SelectedGraphics = "<None>";
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::BeginCombo("Graphics", m_SelectedGraphics.c_str()))
    {
        for (const std::string& s : Parser::graphics | std::ranges::views::keys)
        {
            if (std::ranges::contains(Parser::tilesets, s))
                continue;

            if (ImGui::MenuItem(s.c_str()))
                m_SelectedGraphics = s;
        }

        ImGui::EndCombo();
    }

    for (size_t i = 0; i < Parser::tilesets.size(); i++)
    {
        ImGui::PushID(&i + i);
        if (ImGui::Button("-"))
        {
            Parser::DeleteTileset(i);
            ImGui::PopID();
            break;
        }

        ImGui::SameLine();

        ImGui::Text("%02zu : %s", i, Parser::tilesets[i].c_str());
        ImGui::PopID();
    }
}
