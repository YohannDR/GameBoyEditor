#pragma once

#include "ui_window.hpp"

class PhysicsEditor : public UiWindow
{
public:
    explicit PhysicsEditor() { name = "Physics editor"; }

    void Update() override;
};
