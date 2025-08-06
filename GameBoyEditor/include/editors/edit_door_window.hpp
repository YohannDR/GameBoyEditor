#pragma once

#include "door.hpp"
#include "ui_window.hpp"

class EditDoorWindow : public UiWindow
{
public:
    explicit EditDoorWindow() { name = "Edit door"; }

    void Setup(Door* door);

    void Update() override;

private:
    Door* m_Door = nullptr;
};
