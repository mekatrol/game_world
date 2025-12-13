#pragma once

#include "util/texture.hpp"
#include <glm/vec2.hpp>
#include <string>
#include <utility>

namespace util
{
    class SpriteSheet
    {
    public:
        SpriteSheet() = default;

        SpriteSheet(const std::string &path, int sprite_width, int sprite_height)
        {
            load_from_file(path, sprite_width, sprite_height);
        }

        bool load_from_file(const std::string &path, int sprite_width, int sprite_height)
        {
            m_sprite_width = sprite_width;
            m_sprite_height = sprite_height;
            return m_texture.load_from_file(path) && validate();
        }

        void bind(GLuint slot = 0) const { m_texture.bind(slot); }
        void unbind() const { m_texture.unbind(); }

        [[nodiscard]] const Texture &texture() const noexcept { return m_texture; }
        [[nodiscard]] Texture &texture() noexcept { return m_texture; }

        [[nodiscard]] int sprite_width() const noexcept { return m_sprite_width; }
        [[nodiscard]] int sprite_height() const noexcept { return m_sprite_height; }

        [[nodiscard]] int columns() const noexcept
        {
            return (m_sprite_width > 0) ? (m_texture.width() / m_sprite_width) : 0;
        }

        [[nodiscard]] int rows() const noexcept
        {
            return (m_sprite_height > 0) ? (m_texture.height() / m_sprite_height) : 0;
        }

        [[nodiscard]] int sprite_count() const noexcept
        {
            return columns() * rows();
        }

        // Returns UV rectangle: (uv_min, uv_max) in normalized 0..1
        [[nodiscard]] std::pair<glm::vec2, glm::vec2> uv_rect(int index) const
        {
            const int cols = columns();
            if (cols <= 0 || m_sprite_width <= 0 || m_sprite_height <= 0)
            {
                return {glm::vec2(0.0f), glm::vec2(1.0f)};
            }

            const int x = index % cols;
            const int y = index / cols;

            const float tex_w = static_cast<float>(m_texture.width());
            const float tex_h = static_cast<float>(m_texture.height());

            const float u0 = (x * m_sprite_width) / tex_w;
            const float v0 = (y * m_sprite_height) / tex_h;
            const float u1 = ((x + 1) * m_sprite_width) / tex_w;
            const float v1 = ((y + 1) * m_sprite_height) / tex_h;

            return {glm::vec2(u0, v0), glm::vec2(u1, v1)};
        }

    private:
        [[nodiscard]] bool validate() const
        {
            if (m_texture.id() == 0 || m_texture.width() <= 0 || m_texture.height() <= 0)
            {
                return false;
            }

            if (m_sprite_width <= 0 || m_sprite_height <= 0)
            {
                return false;
            }

            // Optional: require clean divisibility (common for sprite sheets).
            // If you want to allow partial tiles, remove this.
            if ((m_texture.width() % m_sprite_width) != 0)
            {
                return false;
            }
            if ((m_texture.height() % m_sprite_height) != 0)
            {
                return false;
            }

            return true;
        }

    private:
        Texture m_texture{};
        int m_sprite_width{};
        int m_sprite_height{};
    };
} // namespace util
