#pragma once

#include <cmath>

#include "core.hpp"
#include "texture.hpp"
#include "imgui/imgui.h"

class RenderTarget
{
public:
    explicit RenderTarget(int32_t width, int32_t height);

    void SetSize(int32_t width, int32_t height);
    void Bind() const;
    void Unbind() const;
    void Render() const;

    _NODISCARD const Texture& GetTexture() const;
    _NODISCARD ImVec2 GetSize() const;

    float_t scale = 1.f;

private:
    void CreateQuad();

    Texture m_Texture;
    uint32_t m_Fbo;

    uint32_t m_Vao;
    uint32_t m_Vbo;

    int32_t m_Width;
    int32_t m_Height;
};
