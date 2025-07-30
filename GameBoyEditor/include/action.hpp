#pragma once

#include "core.hpp"

#include <string>

class Action
{
public:
    explicit Action(std::string n) : name(std::move(n)) {}
    virtual ~Action() = default;

    DEFAULT_COPY_MOVE_OPERATIONS(Action)

    std::string name;

    virtual void Do() = 0;
    virtual void Undo() = 0;
};
