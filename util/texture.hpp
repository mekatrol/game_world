#pragma once

#include <string>
#include <glad/gl.h>

namespace util
{
    // Simple 2D texture wrapper.
    // Loads PNG/JPG/etc via stb_image into an OpenGL texture.
    class Texture
    {
    public:
        Texture() = default;
        explicit Texture(const std::string &path, bool flip);

        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

        Texture(Texture &&other) noexcept;
        Texture &operator=(Texture &&other) noexcept;

        bool load_from_file(const std::string &path, bool flip);

        void set_filtering(GLenum min_filter, GLenum mag_filter);

        void bind(GLuint slot = 0) const;
        void unbind() const;

        GLuint id() const noexcept { return m_texture_id; }
        int width() const noexcept { return m_width; }
        int height() const noexcept { return m_height; }
        void release();

    private:
        GLuint m_texture_id{};
        int m_width{};
        int m_height{};
    };
}
