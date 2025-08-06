#pragma once

#include "parser.hpp"
#include "ui_window.hpp"

class AddResource : public UiWindow
{
public:
    explicit AddResource() { name = "Add resource"; hasUndoRedo = false; }

    void Setup(SymbolType type);

    void Update() override;

private:
    std::string m_SymbolName;
    std::string m_FileName;

    std::string m_FullFilePath;

    uint8_t m_RoomCollisionTable = 0;
    uint8_t m_RoomOriginX = 0;
    uint8_t m_RoomOriginY = 0;

    void NormalSymbolFields();
    void RoomFields();

    void CreateResource() const;
    void CreateGraphics() const;
    void CreateAnimation() const;
    void CreateRoom();

    static void RegenerateRoomIncludeFile();
    static void CreateRoomHeaderFile();

    SymbolType m_Type = SymbolType::Graphics;
};
