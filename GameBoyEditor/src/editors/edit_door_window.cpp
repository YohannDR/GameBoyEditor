#include "editors/edit_door_window.hpp"

#include "parser.hpp"

void EditDoorWindow::Setup(Door* const door)
{
    m_Door = door;
}

void EditDoorWindow::Update()
{
    const size_t doorId = Parser::GetDoorId(*m_Door);
    ImGui::Text("Door id : %zu", doorId);
    ImGui::Text("Owner room : %d", m_Door->ownerRoom);

    constexpr uint8_t minDoor = 0;
    const uint8_t maxDoor = Parser::doors.size() - 1;
    ImGui::SliderScalar("Destination door", ImGuiDataType_U8, &m_Door->targetDoor, &minDoor, &maxDoor);

    constexpr uint8_t minSize = 1;
    constexpr uint8_t maxSize = 10;
    ImGui::SliderScalar("Width", ImGuiDataType_U8, &m_Door->width, &minSize, &maxSize);
    ImGui::SliderScalar("Height", ImGuiDataType_U8, &m_Door->height, &minSize, &maxSize);

    bool_t loadsTileset = m_Door->tileset != 255;
    if (ImGui::Checkbox("Loads tileset", &loadsTileset))
    {
        if (loadsTileset)
            m_Door->tileset = 0;
        else
            m_Door->tileset = 255;
    }

    if (loadsTileset)
    {
        if (ImGui::BeginCombo("Tileset to load", Parser::tilesets[m_Door->tileset].c_str()))
        {
            for (size_t i = 0; i < Parser::tilesets.size(); i++)
            {
                if (ImGui::MenuItem(Parser::tilesets[i].c_str()))
                    m_Door->tileset = i;
            }

            ImGui::EndCombo();
        }
    }

    if (m_Door->targetDoor == doorId)
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "WARNING : Destination door is the same as self");
}
