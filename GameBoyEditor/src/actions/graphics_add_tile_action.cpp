#include "actions/graphics_add_tile_action.hpp"

GraphicsAddTileAction::GraphicsAddTileAction(Graphics* const graphics, const size_t position)
    : Action("Delete tile"), m_Graphics(graphics), m_Position(position)
{
    for (size_t i = 0; i < 16; i++)
        m_Tile[i] = (*m_Graphics)[m_Position * 16 + i];
}

void GraphicsAddTileAction::Do()
{
    m_Graphics->insert_range(m_Graphics->begin() + static_cast<int64_t>(m_Position) * 16, m_Tile);
}

void GraphicsAddTileAction::Undo()
{
    m_Graphics->erase(m_Graphics->begin() + static_cast<int64_t>(m_Position) * 16, m_Graphics->begin() + static_cast<int64_t>(m_Position + 1) * 16);
}
