#include "editors/animation_editor.hpp"

#include <functional>
#include <iostream>
#include <ranges>

#include "parser.hpp"
#include "ui.hpp"

AnimationEditor::AnimationEditor()
{
    name = "Animation editor";

    for (RenderTarget& renderTarget : m_SpriteRenderTargets)
    {
        renderTarget.Create(8, 8);
        renderTarget.scale = 5.f;
    }

    m_GraphicsRenderTarget.Create(8 * 16, 8);
    m_GraphicsRenderTarget.scale = 4;
}

void AnimationEditor::Update()
{
    ImGui::SeparatorText("Selection");
    DrawAnimationSelector();
    DrawGraphicsSelector();

    if (m_SelectedAnimation == "<None>" || m_SelectedGraphics == "<None>")
        return;

    Ui::CreateSubWindow("options", ImGuiChildFlags_ResizeX | ImGuiChildFlags_AutoResizeY, ImVec2(ImGui::GetContentRegionAvail().x / 2, 0));
    DrawPlaybackControls();

    ImGui::BeginDisabled(m_Playing);
    DrawFrameInfo();
    DrawPartInfo();
    ImGui::EndDisabled();
    ImGui::EndChild();

    UpdatePlayback();

    ImGui::SameLine();
    ImGui::BeginGroup();
    DrawGraphics();
    DrawOam();
    ImGui::EndGroup();
}

void AnimationEditor::Setup(const std::string& animation, const std::string& graphics)
{
    m_SelectedAnimation = animation;
    m_SelectedGraphics = graphics;
}

void AnimationEditor::DrawAnimationSelector()
{
    if (!ImGui::BeginCombo("Animation", m_SelectedAnimation.c_str()))
        return;

    for (const std::string& s : Parser::animations | std::ranges::views::keys)
    {
        if (ImGui::MenuItem(s.c_str()))
            m_SelectedAnimation = s;
    }

    ImGui::EndCombo();
}

void AnimationEditor::DrawGraphicsSelector()
{
    if (!ImGui::BeginCombo("Graphics", m_SelectedGraphics.c_str()))
        return;

    for (const std::string& s : Parser::graphics | std::ranges::views::keys)
    {
        if (ImGui::MenuItem(s.c_str()))
        {
            m_SelectedGraphics = s;
            
            const Graphics& gfx = Parser::graphics[s];
            const size_t tileMax = gfx.size() / 16;
            m_GraphicsRenderTarget.SetSize(16 * 8, static_cast<int32_t>((1 + tileMax / 16) * 8));
        }
    }

    ImGui::EndCombo();
}

void AnimationEditor::DrawPlaybackControls()
{
    ImGui::SeparatorText("Controls");

    ImGui::SliderFloat("Zoom", &m_OamPixelSize, 5.f, 15.f);
    for (RenderTarget& renderTarget : m_SpriteRenderTargets)
        renderTarget.scale = m_OamPixelSize;
    
    ImGui::Checkbox("Draw origin", &m_DrawOrigin);

    if (ImGui::Button(m_Playing ? "Stop" : "Play"))
    {
        m_AnimationTimer = 0;
        m_CurrentFrame = 0;
        m_SelectedPart = 0;
        m_Playing ^= true;
    }
}

void AnimationEditor::DrawFrameInfo()
{
    ImGui::SeparatorText("Frame");
    ImGui::PushID("Frame");

    constexpr uint8_t zero = 0;
    Animation& animation = Parser::animations[m_SelectedAnimation];
    const uint8_t nbrFrames = static_cast<uint8_t>(animation.size() - 1);

    ImGui::SliderScalar("Current frame", ImGuiDataType_U8, &m_CurrentFrame, &zero, &nbrFrames);

    ImGui::BeginDisabled(nbrFrames >= 85);
    if (ImGui::Button("+"))
    {
        animation.insert(animation.begin() + m_CurrentFrame, AnimationFrame{})->oam.emplace_back();
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(nbrFrames == 0);
    if (ImGui::Button("-"))
    {
        animation.erase(animation.begin() + m_CurrentFrame);

        if (m_CurrentFrame == nbrFrames)
            m_CurrentFrame--;
    }
    ImGui::EndDisabled();

    ImGui::InputScalar("Duration", ImGuiDataType_U8, &animation[m_CurrentFrame].duration);
    ImGui::PopID();
}

void AnimationEditor::DrawPartInfo()
{
    ImGui::SeparatorText("Part");
    ImGui::PushID("Part");

    constexpr uint8_t zero = 0;
    Animation& animation = Parser::animations[m_SelectedAnimation];
    std::vector<OamEntry>& entries = animation[m_CurrentFrame].oam;

    ImGui::BeginDisabled(entries.size() == 10);
    if (ImGui::Button("+"))
        entries.insert(entries.begin() + m_SelectedPart, OamEntry{});
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(entries.size() == 1);
    if (ImGui::Button("-"))
    {
        entries.erase(entries.begin() + m_SelectedPart);

        if (m_SelectedPart == entries.size())
            m_SelectedPart--;
    }
    ImGui::EndDisabled();

    const uint8_t nbrParts = static_cast<uint8_t>(entries.size());

    const uint8_t partMax = nbrParts - 1;
    ImGui::SliderScalar("Current part", ImGuiDataType_U8, &m_SelectedPart, &zero, &partMax);
    
    OamEntry& entry = entries[m_SelectedPart];

    ImGui::DragScalar("Y", ImGuiDataType_S8, &entry.y);
    ImGui::DragScalar("X", ImGuiDataType_S8, &entry.x);
    ImGui::DragScalar("Tile", ImGuiDataType_U8, &entry.tileIndex);

    const std::function<void(const char_t*, uint8_t)> editField = [&entry](const char_t* const label, const uint8_t flag) -> void
    {
        bool_t value = entry.properties & flag; 
        ImGui::Checkbox(label, &value);
        if (value)
            entry.properties |= flag;
        else
            entry.properties &= ~flag;
    };

    editField("X-flip", 1u << 5);
    ImGui::SameLine();
    editField("Y-flip", 1u << 6);
    ImGui::SameLine();
    editField("Low prio", 1u << 7);
    ImGui::SameLine();
    editField("Palette", 1u << 4);

    ImGui::PopID();
}

void AnimationEditor::DrawGraphics()
{
    const Graphics& graphics = Parser::graphics[m_SelectedGraphics];

    Ui::CreateSubWindow("graphics", ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY);
    ImGui::SliderFloat("Zoom", &m_GraphicsRenderTarget.scale, 4, 16);
    Ui::DrawGraphics(m_GraphicsRenderTarget, graphics, m_ColorPalette, &m_SelectedTile);
    ImGui::EndChild();
}

void AnimationEditor::DrawOam()
{
    const Graphics& graphics = Parser::graphics[m_SelectedGraphics];
    const Animation& animation = Parser::animations[m_SelectedAnimation];

    Ui::CreateSubWindow("oam", ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY);

    const ImVec2 size = ImGui::GetContentRegionAvail();
    const ImVec2 pos = ImGui::GetWindowPos();
    const ImVec2 middleAbs = ImVec2(pos.x + size.x / 2 - ImGui::GetScrollX(), pos.y + size.y / 2 - ImGui::GetScrollY());
    const ImVec2 middle = ImVec2(size.x / 2, size.y / 2);

    if (m_DrawOrigin)
        Ui::DrawCross(ImVec2(middleAbs.x - 4, middleAbs.y - 4), 2);

    for (size_t i = 0; i < animation[m_CurrentFrame].oam.size(); i++)
    {
        ImGui::PushID(&i + i);
        const OamEntry& entry = animation[m_CurrentFrame].oam[i];
        const ImVec2 position = ImVec2(entry.x * m_OamPixelSize, entry.y * m_OamPixelSize);

        if (entry.tileIndex < graphics.size() / 16)
        {
            ImGui::SetCursorPos(ImVec2(middle.x + position.x, middle.y + position.y));
            Ui::DrawTile(m_SpriteRenderTargets[i], graphics, entry.tileIndex, m_ColorPalette);

            if (ImGui::IsItemClicked())
                m_SelectedPart = i;
        }
        else
        {
            Ui::DrawCross(ImVec2(middleAbs.x + position.x, middleAbs.y + position.y), m_OamPixelSize);
        }
        ImGui::PopID();
    }

    const ImVec2 p1 = ImVec2(middleAbs.x + animation[m_CurrentFrame].oam[m_SelectedPart].x * m_OamPixelSize, middleAbs.y + animation[m_CurrentFrame].oam[m_SelectedPart].y * m_OamPixelSize);
    const ImVec2 p2 = ImVec2(p1.x + 8 * m_OamPixelSize, p1.y + 8 * m_OamPixelSize);
    ImGui::GetWindowDrawList()->AddRect(p1, p2, IM_COL32(0xFF, 0x00, 0x00, 0xFF));

    ImGui::EndChild();
}

void AnimationEditor::UpdatePlayback()
{
    if (!m_Playing)
        return;

    const Animation& animation = Parser::animations[m_SelectedAnimation];

    m_AnimationTimer++;

    if (m_AnimationTimer >= animation[m_CurrentFrame].duration)
    {
        m_AnimationTimer = 0;
        m_CurrentFrame++;

        if (m_CurrentFrame == animation.size())
            m_CurrentFrame = 0;
    }
}
