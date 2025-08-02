#pragma once

#include "core.hpp"

class Texture
{
public:
    void Create();
    void SetData(int32_t internalFormat, int32_t inputFormat, int32_t width, int32_t height, const void* data) const;

    void Bind() const;
    void Unbind() const;

    void BindToActive(uint32_t index) const;
    void UnbindToActive(uint32_t index) const;

    _NODISCARD uint32_t GetId() const;

private:
    uint32_t m_TextureId = 0;
};
