#include "editors/add_resource.hpp"

#include "application.hpp"
#include "imgui/imgui_stdlib.h"
#include "magic_enum/magic_enum.hpp"

void AddResource::Setup(const SymbolType type)
{
    m_Type = type;
    m_FileName = "";
    m_SymbolName = "";
}

void AddResource::Update()
{
    ImGui::Text("Adding %*s", static_cast<int32_t>(magic_enum::enum_name(m_Type).size()), magic_enum::enum_name(m_Type).data());

    ImGui::InputText("File name", &m_FileName);
    ImGui::SetItemTooltip("In which file the new resource should be put");
    ImGui::InputText("Symbol name", &m_SymbolName);
    ImGui::SetItemTooltip("The name of the resource, should start with an underscore 's'");

    m_FullFilePath = Application::projectPath + '\\' + m_FileName;

    const bool_t fileExists = std::filesystem::exists(m_FullFilePath) && std::filesystem::is_regular_file(m_FullFilePath);

    const bool_t symbolAlreadyExists = std::ranges::contains(Parser::existingSymbols, m_SymbolName);
    const bool_t symbolPrefix = m_SymbolName[0] == 's';
    const bool_t nameValid = !m_SymbolName.empty() && symbolPrefix && !symbolAlreadyExists;

    if (!fileExists)
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "File is invalid");

    if (symbolAlreadyExists)
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "A symbol with this name already exists");

    if (!symbolPrefix)
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Symbol name should start with an underscore 's'");

    ImGui::BeginDisabled(!fileExists || !nameValid);
    if (ImGui::Button("Add"))
    {
        CreateResource();
        open = false;
    }
    ImGui::EndDisabled();
}

void AddResource::CreateResource() const
{
    switch (m_Type)
    {
        case SymbolType::Graphics: CreateGraphics(); break;
        case SymbolType::Animation: CreateAnimation(); break;
        case SymbolType::RoomData:
            break;

        case SymbolType::Tilemap:
        case SymbolType::Clipdata:
        case SymbolType::SpriteData:
            return;
    }

    Parser::fileAssociations[m_FullFilePath].emplace_back(m_Type, m_SymbolName);
}

void AddResource::CreateGraphics() const
{
    const std::vector<uint8_t> dummyGraphics(16);
    Parser::graphics[m_SymbolName] = dummyGraphics;
}

void AddResource::CreateAnimation() const
{
    Animation dummyAnimation(1);
    dummyAnimation[0].oam.emplace_back(0, 0, 0, 0);
    dummyAnimation[0].duration = 60;
    Parser::animations[m_SymbolName] = dummyAnimation;
}
