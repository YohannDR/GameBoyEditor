#include "actions/plot_pixel_action.hpp"

void PlotPixelAction::Do()
{
    for (const PixelEdit& edit : m_Edits)
    {
        (*m_Graphics)[edit.tile * 16 + edit.row * 2 + 0] = edit.newPlane0;
        (*m_Graphics)[edit.tile * 16 + edit.row * 2 + 1] = edit.newPlane1;
    }
}

void PlotPixelAction::Undo()
{
    for (const PixelEdit& edit : m_Edits)
    {
        (*m_Graphics)[edit.tile * 16 + edit.row * 2 + 0] = edit.oldPlane0;
        (*m_Graphics)[edit.tile * 16 + edit.row * 2 + 1] = edit.oldPlane1;
    }
}

void PlotPixelAction::AddEdit(const uint8_t tile, const uint8_t row, const uint8_t oldPlane0, const uint8_t oldPlane1, const uint8_t newPlane0, const uint8_t newPlane1)
{
    for (PixelEdit& edit : m_Edits)
    {
        if (edit.tile == tile && edit.row == row)
        {
            edit.newPlane0 = newPlane0;
            edit.newPlane1 = newPlane1;
            return;
        }
    }

    m_Edits.emplace_back(tile, row, oldPlane0, oldPlane1, newPlane0, newPlane1);
}
