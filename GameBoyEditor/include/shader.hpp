#pragma once

#include <filesystem>

class Shader
{
public:
    void Load(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
    void Use() const;

    void SetUniform(const std::string& name, int32_t value) const;

private:
    static void CheckCompileErrors(uint32_t shader, const std::string& type);

    uint32_t m_Handle = 0;
};
