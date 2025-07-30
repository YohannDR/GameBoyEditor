#pragma once

#include "action.hpp"
#include "parser.hpp"

class GraphicsAddTileAction : public Action
{
public:
    explicit GraphicsAddTileAction(Graphics* graphics, size_t position);

    void Do() override;
    void Undo() override;

private:
    Graphics* m_Graphics;
    size_t m_Position;
    std::array<uint8_t, 16> m_Tile;
};
