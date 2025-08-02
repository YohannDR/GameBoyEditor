#include "texture.hpp"

#include <functional>

#include "glad/glad.h"

void Texture::Create()
{
    glGenTextures(1, &m_TextureId);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

void Texture::SetData(const int32_t internalFormat, const int32_t inputFormat, const int32_t width, const int32_t height, const void* const data) const
{
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, inputFormat, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind() const
{
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
}

void Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::BindToActive(const uint32_t index) const
{
    glActiveTexture(GL_TEXTURE0 + index);
    Bind();
}

void Texture::UnbindToActive(const uint32_t index) const
{
    glActiveTexture(GL_TEXTURE0 + index);
    Unbind();
}

uint32_t Texture::GetId() const { return m_TextureId; }
