#include "ui_window.hpp"

void UiWindow::ProcessUndoRedo()
{
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z))
        m_ActionQueue.StepBack();

    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y))
        m_ActionQueue.StepForward();
}
