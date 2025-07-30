#include "actions/edit_tilemap_action.hpp"

void EditTilemapAction::Do()
{
    for (const BlockEdit& edit : m_Edits)
        (*m_Tilemap)[edit.y][edit.x] = edit.newValue;
}

void EditTilemapAction::Undo()
{
    for (const BlockEdit& edit : m_Edits)
        (*m_Tilemap)[edit.y][edit.x] = edit.oldValue;
}

void EditTilemapAction::AddEdit(const uint8_t x, const uint8_t y, const uint8_t oldValue, const uint8_t newValue)
{
    for (const BlockEdit& edit : m_Edits)
    {
        if (edit.x == x && edit.y == y)
            return;
    }
    
    m_Edits.emplace_back(x, y, oldValue, newValue);
}
