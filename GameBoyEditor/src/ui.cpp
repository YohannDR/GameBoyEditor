#include "ui.hpp"

#include <iostream>

#include "application.hpp"
#include "editors/animation_editor.hpp"
#include "editors/edit_sprite_window.hpp"
#include "editors/graphics_editor.hpp"
#include "editors/room_editor.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

#include "glad/glad.h"

void Ui::Init()
{
    SetupWindow();
    SetupRenderer();
}

void Ui::MainMenuBar()
{
    bool_t openPopup = false;

    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Locate project"))
            openPopup = true;

        ImGui::BeginDisabled(!Application::IsProjectLoaded());
        if (ImGui::MenuItem("Build"))
        {
            Parser::Save();
            Application::BuildRom(true);
        }

        if (ImGui::MenuItem("Save"))
        {
            Parser::Save();
        }

        ImGui::EndDisabled();

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
            ShowWindow<AnimationEditor>();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Add"))
    {
        if (ImGui::MenuItem("Graphics"))
        {
            
        }

        if (ImGui::MenuItem("Room"))
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
        if (!w->open)
            continue;

        if (ImGui::Begin(w->name.c_str(), w->canBeClosed ? &w->open : nullptr, ImGuiWindowFlags_MenuBar))
        {
            w->Update();
            w->ProcessShortcuts();
            w->DrawMenuBar();
        }

        ImGui::End();
    }
}

void Ui::OnProjectLoaded()
{
    for (UiWindow* const w : m_Windows)
        w->OnProjectLoaded();
}

void Ui::DrawPalette(Palette& palette, const float_t size, size_t* selectedColor)
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

        const ImVec2 p1 = ImVec2(ImGui::GetWindowPos().x + x, ImGui::GetWindowPos().y + y + 1);
        const ImVec2 p2 = ImVec2(p1.x + size, p1.y + size + spacing - 1);

        dl->AddRectFilled(p1, p2, rgb);
        dl->AddRect(ImVec2(p1.x - 1.f, p1.y - 1.f), ImVec2(p2.x + 1.f, p2.y + 1.f),
            selectedColor && *selectedColor == i
            ? IM_COL32(0x00, 0xFF, 0x00, 0xFF)
            : IM_COL32(0xFF, 0x00, 0x00, 0xFF));

        ImGui::SetCursorPos(buttonPos);
        if (ImGui::InvisibleButton("btn", ImVec2(p2.x - p1.x, p2.y - p1.y)))
        {
            if (selectedColor)
                *selectedColor = i;
        }

        if (i != 3)
            ImGui::SetCursorPos(ImVec2(buttonPos.x + size + 10, buttonPos.y));

        ImGui::PopID();
    }

    if (selectedColor)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_A))
            *selectedColor = 0;
        else if (ImGui::IsKeyPressed(ImGuiKey_Z))
            *selectedColor = 1;
        else if (ImGui::IsKeyPressed(ImGuiKey_E))
            *selectedColor = 2;
        else if (ImGui::IsKeyPressed(ImGuiKey_R))
            *selectedColor = 3;
    }
}

void Ui::DrawTile(const RenderTarget& renderTarget, const std::vector<uint8_t>& graphics, const size_t graphicsIndex, const Palette& palette)
{
    const std::vector<uint8_t> tileData{graphics.begin() + graphicsIndex * 16, graphics.begin() + graphicsIndex * 16 + 16};

    m_GraphicsTexture.SetData(GL_RG8, GL_RG, 16, 1, tileData.data());
    m_GraphicsTexture.BindToActive(0);

    m_GraphicsShader.Use();
    m_GraphicsShader.SetUniform("graphics", 0);
    m_GraphicsShader.SetUniform("gfxSize", static_cast<int32_t>(tileData.size()));
    m_GraphicsShader.SetUniform("colors[0]", GetRgbColorVec(palette[0]));
    m_GraphicsShader.SetUniform("colors[1]", GetRgbColorVec(palette[1]));
    m_GraphicsShader.SetUniform("colors[2]", GetRgbColorVec(palette[2]));
    m_GraphicsShader.SetUniform("colors[3]", GetRgbColorVec(palette[3]));

    renderTarget.Render();
    renderTarget.Draw();
}

void Ui::DrawCross(const ImVec2 position, const float_t size)
{
    ImDrawList* const dl = ImGui::GetWindowDrawList();

    dl->AddLine(position, ImVec2(position.x + size * 8, position.y + size * 8), IM_COL32(0xFF, 0x00, 0x00, 0xFF));
    dl->AddLine(ImVec2(position.x + size * 8, position.y), ImVec2(position.x, position.y + size * 8), IM_COL32(0xFF, 0x00, 0x00, 0xFF));
}

void Ui::DrawGraphics(const RenderTarget& renderTarget, const std::vector<uint8_t>& graphics, const Palette& palette, size_t* const selectedTile)
{
    const size_t tileAmount = graphics.size() / 16;

    const ImVec2 position = GetPosition();

    const int32_t tileCount = static_cast<int32_t>(graphics.size());
    m_GraphicsTexture.SetData(GL_RG8, GL_RG, tileCount, 1, graphics.data());
    m_GraphicsTexture.BindToActive(0);

    m_GraphicsShader.Use();
    m_GraphicsShader.SetUniform("graphics", 0);
    m_GraphicsShader.SetUniform("gfxSize", tileCount);
    m_GraphicsShader.SetUniform("colors[0]", GetRgbColorVec(palette[0]));
    m_GraphicsShader.SetUniform("colors[1]", GetRgbColorVec(palette[1]));
    m_GraphicsShader.SetUniform("colors[2]", GetRgbColorVec(palette[2]));
    m_GraphicsShader.SetUniform("colors[3]", GetRgbColorVec(palette[3]));

    renderTarget.Render();
    renderTarget.Draw();

    const float_t pixelSize = renderTarget.scale;
    const float_t tileMax = static_cast<float_t>(tileAmount);
    const size_t index = DrawSelectSquare(position, ImVec2(tileMax >= 16 ? 16 : tileMax, 1 + tileMax / 16), pixelSize * 8);

    const ImVec2 selectPos = ImVec2(
        position.x + static_cast<float_t>(*selectedTile % 16) * pixelSize * 8,
        position.y + static_cast<float_t>(*selectedTile / 16) * pixelSize * 8  // NOLINT(bugprone-integer-division)
    );
    ImGui::GetWindowDrawList()->AddRect(selectPos, ImVec2(selectPos.x + pixelSize * 8, selectPos.y + pixelSize * 8), IM_COL32(0xFF, 0x00, 0x00, 0xFF));

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && index != std::numeric_limits<size_t>::max() && index < tileAmount)
        *selectedTile = index;
}

size_t Ui::DrawSelectSquare(const ImVec2 position, const ImVec2 size, const float_t squareSize)
{
    const ImVec2 mousePos = ImGui::GetMousePos();
    const ImVec2 delta = ImVec2(mousePos.x - position.x, mousePos.y - position.y);
    size_t index = std::numeric_limits<size_t>::max();

    if (delta.x >= 0 && delta.x < size.x * squareSize && delta.y >= 0 && delta.y < size.y * squareSize)
    {
        const size_t x = static_cast<size_t>(delta.x) / static_cast<size_t>(squareSize);
        const size_t y = static_cast<size_t>(delta.y) / static_cast<size_t>(squareSize);

        index = y * static_cast<size_t>(size.x) + x;

        const ImVec2 p1 = ImVec2(position.x + static_cast<float_t>(x) * squareSize, position.y + static_cast<float_t>(y) * squareSize);
        const ImVec2 p2 = ImVec2(p1.x + squareSize, p1.y + squareSize);

        ImGui::GetWindowDrawList()->AddRect(p1, p2, IM_COL32(0xFF, 0x00, 0x00, 0xFF));
    }

    return index;
}

void Ui::CreateSubWindow(const char_t* const name, const ImGuiChildFlags flags, const ImVec2 size, const uint32_t bgColor)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
    ImGui::BeginChild(name, size, ImGuiChildFlags_Borders | flags, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void Ui::SetupWindow()
{
    m_Windows.push_back(new GraphicsEditor());
    m_Windows.push_back(new RoomEditor());
    m_Windows.push_back(new EditSpriteWindow());
    m_Windows.push_back(new AnimationEditor());

    ShowWindow<RoomEditor>();
}

void Ui::SetupRenderer()
{
    m_GraphicsShader.Load("shaders/graphics.vert", "shaders/graphics.frag");

    m_GraphicsTexture.Create();
}

ImVec2 Ui::GetPosition()
{
    return ImVec2(
        ImGui::GetWindowPos().x + ImGui::GetCursorPosX() - ImGui::GetScrollX(),
        ImGui::GetWindowPos().y + ImGui::GetCursorPosY() - ImGui::GetScrollY()
    );
}
