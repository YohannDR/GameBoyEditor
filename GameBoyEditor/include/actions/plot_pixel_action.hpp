#pragma once

#pragma once

#include <vector>

#include "action.hpp"
#include "parser.hpp"

class PlotPixelAction : public Action
{
public:
    explicit PlotPixelAction(Graphics* const graphics) : Action("Edit graphics"), m_Graphics(graphics) {}

    void Do() override;
    void Undo() override;

    void AddEdit(uint8_t tile, uint8_t row, uint8_t oldPlane0, uint8_t oldPlane1, uint8_t newPlane0, uint8_t newPlane1);

private:
    struct PixelEdit
    {
        uint8_t tile;
        uint8_t row;
        uint8_t oldPlane0;
        uint8_t oldPlane1;
        uint8_t newPlane0;
        uint8_t newPlane1;
    };

    std::vector<PixelEdit> m_Edits {};
    Graphics* m_Graphics;
};

