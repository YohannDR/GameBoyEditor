#include "ui_window.hpp"

#include "parser.hpp"

void UiWindow::ProcessShortcuts()
{
    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z))
        m_ActionQueue.StepBack();

    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y))
        m_ActionQueue.StepForward();

    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S))
        Parser::Save();
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
