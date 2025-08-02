#pragma once

#include <cmath>

#include "core.hpp"
#include "texture.hpp"
#include "imgui/imgui.h"

class RenderTarget
{
public:
    void Create(int32_t width, int32_t height);

    void SetSize(int32_t width, int32_t height);
    void Bind() const;
    void Unbind() const;
    void Render() const;
    void Draw() const;

    _NODISCARD const Texture& GetTexture() const;
    _NODISCARD ImVec2 GetSize() const;

    float_t scale = 1.f;

private:
    void CreateQuad();

    Texture m_Texture;
    uint32_t m_Fbo = 0;

    uint32_t m_Vao = 0;
    uint32_t m_Vbo = 0;

    int32_t m_Width = 0;
    int32_t m_Height = 0;
};
