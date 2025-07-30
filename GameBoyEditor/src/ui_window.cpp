#include "ui_window.hpp"

void UiWindow::ProcessUndoRedo()
{
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z))
        m_ActionQueue.StepBack();

    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y))
        m_ActionQueue.StepForward();
}

void UiWindow::DrawMenuBar()
{
    if (!ImGui::BeginMenuBar())
        return;

    ImGui::BeginDisabled(!m_ActionQueue.CanGoBackward());
    if (ImGui::Button("<"))
        m_ActionQueue.StepBack();
    ImGui::SetItemTooltip("Undo");
    ImGui::EndDisabled();

    ImGui::BeginDisabled(!m_ActionQueue.CanGoForward());
    if (ImGui::Button(">"))
        m_ActionQueue.StepForward();
    ImGui::SetItemTooltip("Redo");
    ImGui::EndDisabled();

    ImGui::EndMenuBar();
}
