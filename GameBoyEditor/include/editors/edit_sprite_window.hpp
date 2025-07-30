#pragma once

#include "core.hpp"
#include "room.hpp"
#include "ui_window.hpp"

class EditSpriteWindow : public UiWindow
{
public:
    explicit EditSpriteWindow() { name = "Edit sprite"; }

    void Setup(SpriteData* sprite);

    void Update() override;

private:
    SpriteData* m_Sprite = nullptr;
};
