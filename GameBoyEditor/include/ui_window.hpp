#pragma once

#include <string>

#include "action_queue.hpp"
#include "core.hpp"
#include "imgui/imgui.h"

class UiWindow
{
public:
    explicit UiWindow() = default;
    virtual ~UiWindow() = default;

    DEFAULT_COPY_MOVE_OPERATIONS(UiWindow)

    virtual void Update() = 0;
    virtual void OnProjectLoaded() {}

    void ProcessShortcuts();
    void DrawMenuBar();

    std::string name;
    bool_t canBeClosed = true;
    bool_t hasUndoRedo = true;
    bool_t open = false;

protected:
    ActionQueue m_ActionQueue;
};
