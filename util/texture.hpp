#pragma once

#include <glad/gl.h>
#include <string>

namespace util
{

    class Texture
    {
    public:
        Texture();
        explicit Texture(const std::string &path);
        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

        Texture(Texture &&other) noexcept;
        Texture &operator=(Texture &&other) noexcept;

        bool load_from_file(const std::string &path);

        void bind(GLuint slot = 0) const;
        void unbind() const;

        [[nodiscard]] GLuint id() const noexcept;
        [[nodiscard]] int width() const noexcept;
        [[nodiscard]] int height() const noexcept;

    private:
        void release();

    private:
        GLuint m_texture_id{};
        int m_width{};
        int m_height{};
    };

} // namespace util
