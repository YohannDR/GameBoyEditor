#pragma once

#include <array>

#include "action.hpp"
#include "core.hpp"

class ActionQueue
{
    static constexpr size_t QueueSize = 30;

public:
    void Push(Action* action, bool_t perform = false);
    void StepForward();
    void StepBack();

    _NODISCARD bool_t IsAtBeginning() const;
    _NODISCARD bool_t IsAtEnd() const;

    _NODISCARD bool_t CanGoForward() const;
    _NODISCARD bool_t CanGoBackward() const;

private:
    std::array<Action*, QueueSize> m_Queue {};
    size_t m_QueueIndex = 0;

    void ShiftActions();
    void RerouteQueue();
};
