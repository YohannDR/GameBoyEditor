#include "render_target.hpp"

#include <array>
#include <functional>
#include <iostream>

#include "glad/glad.h"

RenderTarget::RenderTarget(const int32_t width, const int32_t height)
{
    m_Width = width;
    m_Height = height;

    m_Texture.Create();
    m_Texture.SetData(GL_RGBA32F, GL_RGBA, width, height, nullptr);

    glGenFramebuffers(1, &m_Fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture.GetId(), 0);

    m_Texture.Unbind();
    Unbind();

    CreateQuad();
}

void RenderTarget::SetSize(const int32_t width, const int32_t height)
{
    m_Width = width;
    m_Height = height;

    // m_Texture.SetData(width, height, nullptr);

    Bind();

    m_Texture.SetData(GL_RGBA32F, GL_RGBA, width, height, nullptr);
    m_Texture.Bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture.GetId(), 0);

    m_Texture.Unbind();
    Unbind();
}

void RenderTarget::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
}

void RenderTarget::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::Render() const
{
    Bind();
    glBindVertexArray(m_Vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    Unbind();
}

void RenderTarget::Draw() const
{
    ImGui::Image(m_Texture.GetId(), GetSize(), ImVec2(0, 1), ImVec2(1, 0));
}

const Texture& RenderTarget::GetTexture() const { return m_Texture; }

ImVec2 RenderTarget::GetSize() const
{
    return ImVec2(static_cast<float_t>(m_Width) * scale, static_cast<float_t>(m_Height) * scale); 
}

void RenderTarget::CreateQuad()
{
    constexpr float_t quadVertices[] = {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
         1.0f, -1.0f,
    };

    glGenVertexArrays(1, &m_Vao);
    glGenBuffers(1, &m_Vbo);

    glBindVertexArray(m_Vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float_t), nullptr);
}
