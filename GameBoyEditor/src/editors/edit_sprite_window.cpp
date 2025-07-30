#include "editors/edit_sprite_window.hpp"

#include "parser.hpp"

void EditSpriteWindow::Setup(SpriteData* sprite)
{
    m_Sprite = sprite;
}

void EditSpriteWindow::Update()
{
    if (!m_Sprite)
        return;

    if (ImGui::BeginCombo("Sprite ID", m_Sprite->id.c_str()))
    {
        for (const std::string& id : Parser::spriteIds)
        {
            if (ImGui::Selectable(id.c_str(), id == m_Sprite->id))
                m_Sprite->id = id;
        }

        ImGui::EndCombo();
    }

    ImGui::InputScalar("Part ID", ImGuiDataType_U8, &m_Sprite->part);
}
