#include "editors/animation_editor.hpp"

#include <functional>
#include <ranges>

#include "parser.hpp"
#include "ui.hpp"

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
            m_SelectedGraphics = s;
    }

    ImGui::EndCombo();
}

void AnimationEditor::DrawPlaybackControls()
{
    ImGui::SeparatorText("Controls");

    ImGui::SliderFloat("Zoom", &m_OamPixelSize, 5.f, 15.f);
    ImGui::Checkbox("Draw origin", &m_DrawOrigin);

    if (ImGui::Button(m_Playing ? "Stop" : "Play"))
    {
        m_AnimationTimer = 0;
        m_CurrentFrame = 0;
        m_CurrentPart = 0;
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

    if (ImGui::Button("+"))
        entries.insert(entries.begin() + m_CurrentPart, OamEntry{});

    ImGui::SameLine();
    ImGui::BeginDisabled(entries.size() == 1);
    if (ImGui::Button("-"))
    {
        entries.erase(entries.begin() + m_CurrentPart);

        if (m_CurrentPart == entries.size())
            m_CurrentPart--;
    }
    ImGui::EndDisabled();

    const uint8_t nbrParts = static_cast<uint8_t>(entries.size());

    const uint8_t partMax = nbrParts - 1;
    ImGui::SliderScalar("Current part", ImGuiDataType_U8, &m_CurrentPart, &zero, &partMax);
    
    OamEntry& entry = entries[m_CurrentPart];

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
    const std::vector<uint8_t>& graphics = Parser::graphics[m_SelectedGraphics];

    const size_t height = 1 + graphics.size() / 256;
    Ui::CreateSubWindow("graphics", ImGuiChildFlags_ResizeX, ImVec2(0, height * 8 * 4));
    Ui::DrawGraphics(ImGui::GetWindowPos(), graphics, m_ColorPalette, &m_SelectedTile);
    ImGui::EndChild();
}

void AnimationEditor::DrawOam() const
{
    const std::vector<uint8_t>& graphics = Parser::graphics[m_SelectedGraphics];
    const Animation& animation = Parser::animations[m_SelectedAnimation];

    Ui::CreateSubWindow("oam", ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY);

    const ImVec2 size = ImGui::GetWindowSize();
    const ImVec2 pos = ImGui::GetWindowPos();
    const ImVec2 middle = ImVec2(pos.x + size.x / 2 - ImGui::GetScrollX(), pos.y + size.y / 2 - ImGui::GetScrollY());

    if (m_DrawOrigin)
        Ui::DrawCross(ImVec2(middle.x - 4, middle.y - 4), 2);

    for (const OamEntry& entry : animation[m_CurrentFrame].oam)
    {
        const ImVec2 position = ImVec2(middle.x + entry.x * m_OamPixelSize, middle.y + entry.y * m_OamPixelSize);

        if (entry.tileIndex < graphics.size() / 16)
            Ui::DrawTile(position, graphics, entry.tileIndex, m_ColorPalette, m_OamPixelSize);
        else
            Ui::DrawCross(position, m_OamPixelSize);
    }

    ImGui::Dummy(ImVec2(160 * m_OamPixelSize, 144 * m_OamPixelSize));
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
