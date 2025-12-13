#include "util/texture.hpp"

#include <stdexcept>
#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace util
{

    Texture::Texture() = default;

    Texture::Texture(const std::string &path)
    {
        if (!load_from_file(path))
        {
            throw std::runtime_error("Texture: failed to load: " + path);
        }
    }

    Texture::~Texture()
    {
        release();
    }

    Texture::Texture(Texture &&other) noexcept
    {
        *this = std::move(other);
    }

    Texture &Texture::operator=(Texture &&other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }

        release();

        m_texture_id = other.m_texture_id;
        m_width = other.m_width;
        m_height = other.m_height;

        other.m_texture_id = 0;
        other.m_width = 0;
        other.m_height = 0;

        return *this;
    }

    bool Texture::load_from_file(const std::string &path)
    {
        release();

        stbi_set_flip_vertically_on_load(1);

        int width = 0;
        int height = 0;
        int channels = 0;
        unsigned char *pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!pixels)
        {
            return false;
        }

        GLenum internal_format = GL_RGBA8;
        GLenum data_format = GL_RGBA;

        if (channels == 4)
        {
            internal_format = GL_RGBA8;
            data_format = GL_RGBA;
        }
        else if (channels == 3)
        {
            internal_format = GL_RGB8;
            data_format = GL_RGB;
        }
        else if (channels == 1)
        {
            internal_format = GL_R8;
            data_format = GL_RED;
        }
        else
        {
            stbi_image_free(pixels);
            return false;
        }

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            static_cast<GLint>(internal_format),
            width,
            height,
            0,
            data_format,
            GL_UNSIGNED_BYTE,
            pixels);

        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(pixels);

        m_texture_id = tex;
        m_width = width;
        m_height = height;

        return true;
    }

    void Texture::bind(GLuint slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_texture_id);
    }

    void Texture::unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    GLuint Texture::id() const noexcept
    {
        return m_texture_id;
    }

    int Texture::width() const noexcept
    {
        return m_width;
    }

    int Texture::height() const noexcept
    {
        return m_height;
    }

    void Texture::release()
    {
        if (m_texture_id != 0)
        {
            glDeleteTextures(1, &m_texture_id);
            m_texture_id = 0;
        }
        m_width = 0;
        m_height = 0;
    }

} // namespace util
