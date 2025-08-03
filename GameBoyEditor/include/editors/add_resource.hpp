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

    void NormalSymbolFields();
    void RoomFields();

    void CreateResource() const;
    void CreateGraphics() const;
    void CreateAnimation() const;
    void CreateRoom();

    static void RegenerateRoomIncludeFile();
    void CreateRoomHeaderFile() const;

    std::string m_RoomGraphics;
    bool_t m_AutoCreateGraphics = false;

    SymbolType m_Type = SymbolType::Graphics;
};
