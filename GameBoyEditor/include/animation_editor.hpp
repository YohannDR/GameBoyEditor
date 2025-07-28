#pragma once

#include "animation.hpp"
#include "color.hpp"
#include "ui_window.hpp"

class AnimationEditor : public UiWindow
{
public:
    explicit AnimationEditor() { name = "Animation editor"; }

    void Update() override;

    void Setup(const std::string& animation, const std::string& graphics);
    
private:
    void DrawAnimationSelector();
    void DrawGraphicsSelector();
    void DrawPlaybackControls();
    void DrawFrameInfo();
    void DrawPartInfo();

    void DrawGraphics();
    void DrawOam() const;

    void UpdatePlayback();

    std::string m_SelectedAnimation = "<None>";
    std::string m_SelectedGraphics = "<None>";

    uint8_t m_CurrentFrame = 0;
    uint8_t m_CurrentPart = 0;

    bool_t m_Playing = false;
    uint8_t m_AnimationTimer = 0;

    size_t m_SelectedTile;

    Palette m_ColorPalette = Palette{ Transparent, LightGrey, DarkGrey, Black };
    float_t m_OamPixelSize = 5.f;
    bool_t m_DrawOrigin = true;
};
