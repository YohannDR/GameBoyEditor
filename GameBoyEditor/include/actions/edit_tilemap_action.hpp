#pragma once

#include <vector>

#include "action.hpp"
#include "parser.hpp"

class EditTilemapAction : public Action
{
public:
    explicit EditTilemapAction(Tilemap* const tilemap) : Action("Edit tilemap"), m_Tilemap(tilemap) {}

    void Do() override;
    void Undo() override;

    void AddEdit(uint8_t x, uint8_t y, uint8_t oldValue, uint8_t newValue);

    _NODISCARD size_t GetEditCount() const { return m_Edits.size(); }

private:
    struct BlockEdit
    {
        uint8_t x;
        uint8_t y;
        uint8_t oldValue;
        uint8_t newValue;
    };

    std::vector<BlockEdit> m_Edits {};
    Tilemap* m_Tilemap;
};
