#include "ui.hpp"

#include "application.hpp"
#include "graphics_editor.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

void Ui::Init()
{
    m_Windows.push_back(new GraphicsEditor());
}

void Ui::MainMenuBar()
{
    bool_t openPopup = false;

    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Locate project"))
            openPopup = true;

        if (ImGui::MenuItem("Build"))
            Application::BuildRom(true);

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Editors"))
    {
        if (ImGui::MenuItem("Graphics editor"))
        {
            ShowWindow<GraphicsEditor>();
        }

        if (ImGui::MenuItem("Animation editor"))
        {
            
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();

    if (openPopup)
        ImGui::OpenPopup("Input project path");
    
    if (ImGui::BeginPopupModal("Input project path"))
    {
        if (ImGui::InputText("Project path", &Application::projectPath, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (Application::TryParseProject())
                ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

}

void Ui::DrawWindows()
{
    for (UiWindow* const w : m_Windows)
    {
        if (w->hidden)
            continue;
        
        if (ImGui::Begin(w->name.c_str()))
        {
            w->FetchInfo();
            w->Update();
        }

        ImGui::End();
    }
}

void Ui::DrawPalette(const ImVec2 position, Palette& palette, const float_t size, size_t& selectedColor)
{
    const float_t spacing = ImGui::GetStyle().ItemSpacing.y;
    ImDrawList* const dl = ImGui::GetWindowDrawList();

    const float_t y = ImGui::GetCursorPosY();

    for (size_t i = 0; i < palette.size(); i++)
    {
        ImGui::PushID(&i + i);

        float_t x = ImGui::GetCursorPosX();
        if (ImGui::ArrowButtonEx("U", ImGuiDir_Up, ImVec2(size / 2, size / 2)))
            palette[i] = static_cast<Color>((palette[i] + 1) % 4);

        ImGui::SameLine();
        const ImVec2 buttonPos = ImGui::GetCursorPos();
        ImGui::Dummy(ImVec2(1, 1));

        ImGui::SetCursorPosX(x);

        if (ImGui::ArrowButtonEx("D", ImGuiDir_Down, ImVec2(size / 2, size / 2)))
            palette[i] = static_cast<Color>((palette[i] - 1) % 4);

        ImGui::SameLine();
        x = ImGui::GetCursorPosX();
        ImGui::Dummy(ImVec2(1, 1));

        const uint32_t rgb = GetRgbColor(palette[i]);

        const ImVec2 p1 = ImVec2(position.x + x, position.y + y + 1);
        const ImVec2 p2 = ImVec2(p1.x + size, p1.y + size + spacing - 1);

        dl->AddRectFilled(p1, p2, rgb);
        dl->AddRect(ImVec2(p1.x - 1.f, p1.y - 1.f), ImVec2(p2.x + 1.f, p2.y + 1.f),
            selectedColor == i
            ? IM_COL32(0x00, 0xFF, 0x00, 0xFF)
            : IM_COL32(0xFF, 0x00, 0x00, 0xFF));

        ImGui::SetCursorPos(buttonPos);
        if (ImGui::InvisibleButton("btn", ImVec2(p2.x - p1.x, p2.y - p1.y)))
            selectedColor = i;

        if (i != 3)
            ImGui::SetCursorPos(ImVec2(buttonPos.x + size + 10, buttonPos.y));

        ImGui::PopID();
    }
}

void Ui::DrawTile(const ImVec2 position, const std::vector<uint8_t>& graphics, const size_t graphicsIndex, const Palette& palette, const float_t size)
{
    ImDrawList* const dl = ImGui::GetWindowDrawList();

    for (size_t i = 0; i < 8; i++)
    {
        const uint8_t plane0 = graphics[graphicsIndex + i * 2 + 0];
        const uint8_t plane1 = graphics[graphicsIndex + i * 2 + 1];

        for (size_t j = 0; j < 8; j++)
        {
            const size_t bitIndex = 7 - j;
            const size_t plane0Bit = (plane0 & 1 << bitIndex) >> bitIndex;
            const size_t plane1Bit = (plane1 & 1 << bitIndex) >> bitIndex;
            const size_t pixelIndex = plane0Bit | plane1Bit << 1;

            const uint32_t color = GetRgbColor(palette[pixelIndex]);

            const ImVec2 pos = ImVec2(
                position.x + static_cast<float_t>(j) * size,
                position.y + static_cast<float_t>(i) * size
            );

            dl->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), color);
        }
    }
}
