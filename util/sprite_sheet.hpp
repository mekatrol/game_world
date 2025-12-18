#pragma once

#include "texture.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>

namespace util
{
    // SpriteSheet:
    // - loads a texture
    // - interprets it as a grid of same-sized tiles
    // - provides UV rects for a given sprite index
    class SpriteSheet
    {
    public:
        SpriteSheet() = default;

        SpriteSheet(const std::string &path, int sprite_count_x, int sprite_count_y, bool flip)
        {
            load_from_file(path, sprite_count_x, sprite_count_y, flip);
        }

        bool has_mask() const noexcept { return m_mask_texture.is_valid(); }
        bool has_shadow() const noexcept { return m_shadow_texture.is_valid(); }

        bool load_from_file(const std::string &path, int sprite_count_x, int sprite_count_y, bool flip)
        {
            bool success = m_texture.load_from_file(path, flip);

            if (!success)
            {
                return false;
            }

            m_sprite_width = m_texture.width() / sprite_count_x;
            m_sprite_height = m_texture.height() / sprite_count_y;

            return validate();
        }

        bool load_night_overlays(const std::string &mask_path,
                                 const std::string &shadow_path,
                                 bool flip)
        {
            if (!mask_path.empty() && m_mask_texture.load_from_file(mask_path, flip))
            {
                return false;
            }

            return mask_path.empty() || m_shadow_texture.load_from_file(shadow_path, flip);
        }

        const Texture &texture() const noexcept { return m_texture; }
        const Texture &mask_texture() const noexcept { return m_mask_texture; }
        const Texture &shadow_texture() const noexcept { return m_shadow_texture; }

        Texture &texture() noexcept { return m_texture; }
        Texture &mask_texture() noexcept { return m_mask_texture; }
        Texture &shadow_texture() noexcept { return m_shadow_texture; }

        int sprite_width() const noexcept { return m_sprite_width; }
        int sprite_height() const noexcept { return m_sprite_height; }

        int columns() const noexcept
        {
            return (m_sprite_width > 0) ? (m_texture.width() / m_sprite_width) : 0;
        }

        int rows() const noexcept
        {
            return (m_sprite_height > 0) ? (m_texture.height() / m_sprite_height) : 0;
        }

        int sprite_count() const noexcept
        {
            return columns() * rows();
        }

        // Returns UV rectangle as vec4: (u0, v0, u1, v1) in normalized 0..1
        glm::vec4 uv_rect_vec4(int sprite_index) const
        {
            const int cols = columns();

            if (cols <= 0 || m_sprite_width <= 0 || m_sprite_height <= 0)
            {
                return {0.0f, 0.0f, 1.0f, 1.0f};
            }

            const int x = sprite_index % cols;
            const int y = sprite_index / cols;

            const float tex_w = (float)m_texture.width();
            const float tex_h = (float)m_texture.height();

            const float u0 = (x * m_sprite_width) / tex_w;
            const float v0 = (y * m_sprite_height) / tex_h;
            const float u1 = ((x + 1) * m_sprite_width) / tex_w;
            const float v1 = ((y + 1) * m_sprite_height) / tex_h;

            return {u0, v0, u1, v1};
        }

        glm::vec4 uv_from_grid(int col, int row, int cols, int rows) const
        {
            float u0 = col / (float)cols;
            float v0 = row / (float)rows;
            float u1 = (col + 1) / (float)cols;
            float v1 = (row + 1) / (float)rows;

            return {u0, v0, u1, v1};
        }

    private:
        bool validate() const
        {
            if (m_texture.id() == 0 || m_texture.width() <= 0 || m_texture.height() <= 0)
            {
                return false;
            }
            if (m_sprite_width <= 0 || m_sprite_height <= 0)
            {
                return false;
            }
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
        Texture m_mask_texture{};
        Texture m_shadow_texture{};
        int m_sprite_width{};
        int m_sprite_height{};
    };
}
