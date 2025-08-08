#include "editors/physics_editor.hpp"

#include "parser.hpp"

void PhysicsEditor::Update()
{
    ImGui::Text("Notes :");
    ImGui::Text("Origin is the upper left corner, so up is negative, down is positive, left is negative and right is positive");
    ImGui::Text("Units are in sub-pixel, 4 represents a single pixel, and 32 a full tile");

    ImGui::Separator();

    ImGui::DragScalar("X acceleration", ImGuiDataType_U8, &Parser::physics.xAcceleration);
    ImGui::SetItemTooltip("The acceleration applied to the player when moving horizontally");

    ImGui::DragScalar("X velocity cap", ImGuiDataType_U8, &Parser::physics.xVelocityCap);
    ImGui::SetItemTooltip("The maximum horizontal speed of the player");

    ImGui::DragScalar("Y velocity cap", ImGuiDataType_U8, &Parser::physics.yVelocityCap);
    ImGui::SetItemTooltip("The maximum vertical speed of the player (only applied when falling)");

    ImGui::DragScalar("Gravity upwards", ImGuiDataType_U8, &Parser::physics.gravityUpwards);
    ImGui::SetItemTooltip("The gravity applied when the player is moving up");

    ImGui::DragScalar("Gravity downwards", ImGuiDataType_U8, &Parser::physics.gravityDownwards);
    ImGui::SetItemTooltip("The gravity applied when the player is moving down");

    ImGui::DragScalar("Jumping velocity", ImGuiDataType_S8, &Parser::physics.jumpingVelocity);
    ImGui::SetItemTooltip("The immediate Y velocity applied when jumping");
}
