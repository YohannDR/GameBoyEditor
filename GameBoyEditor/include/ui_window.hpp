#pragma once

#include <string>

#include "core.hpp"
#include "imgui/imgui.h"

class UiWindow
{
public:
    explicit UiWindow() = default;
    virtual ~UiWindow() = default;

    DEFAULT_COPY_MOVE_OPERATIONS(UiWindow)

    virtual void Update() = 0;
    void FetchInfo()
    {
        m_Position = ImGui::GetWindowPos();
    }

    std::string name;
    bool_t hidden = true;

protected:
    ImVec2 m_Position;
};
