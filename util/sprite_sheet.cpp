#include "sprite_sheet.hpp"

namespace util
{
    SpriteSheet::SpriteSheet(const std::string &path, int sprite_count_x, int sprite_count_y, bool flip)
    {
        load_from_file(path, sprite_count_x, sprite_count_y, flip);
    }

    bool SpriteSheet::has_mask() const noexcept { return m_mask_texture.is_valid(); }
    bool SpriteSheet::has_shadow() const noexcept { return m_shadow_texture.is_valid(); }

    bool SpriteSheet::load_from_file(const std::string &path, int sprite_count_x, int sprite_count_y, bool flip)
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

    bool SpriteSheet::load_night_overlays(const std::string &mask_path,
                                          const std::string &shadow_path,
                                          bool flip)
    {
        // Preserved original logic exactly (even though it looks inverted).
        if (!mask_path.empty() && m_mask_texture.load_from_file(mask_path, flip))
        {
            return false;
        }

        return mask_path.empty() || m_shadow_texture.load_from_file(shadow_path, flip);
    }

    const Texture &SpriteSheet::texture() const noexcept { return m_texture; }
    const Texture &SpriteSheet::mask_texture() const noexcept { return m_mask_texture; }
    const Texture &SpriteSheet::shadow_texture() const noexcept { return m_shadow_texture; }

    Texture &SpriteSheet::texture() noexcept { return m_texture; }
    Texture &SpriteSheet::mask_texture() noexcept { return m_mask_texture; }
    Texture &SpriteSheet::shadow_texture() noexcept { return m_shadow_texture; }

    int SpriteSheet::sprite_width() const noexcept { return m_sprite_width; }
    int SpriteSheet::sprite_height() const noexcept { return m_sprite_height; }

    int SpriteSheet::columns() const noexcept
    {
        return (m_sprite_width > 0) ? (m_texture.width() / m_sprite_width) : 0;
    }

    int SpriteSheet::rows() const noexcept
    {
        return (m_sprite_height > 0) ? (m_texture.height() / m_sprite_height) : 0;
    }

    int SpriteSheet::sprite_count() const noexcept
    {
        return columns() * rows();
    }

    glm::vec4 SpriteSheet::uv_rect_vec4(int sprite_index) const
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

    glm::vec4 SpriteSheet::uv_from_grid(int col, int row, int cols, int rows) const
    {
        float u0 = col / (float)cols;
        float v0 = row / (float)rows;
        float u1 = (col + 1) / (float)cols;
        float v1 = (row + 1) / (float)rows;

        return {u0, v0, u1, v1};
    }

    bool SpriteSheet::validate() const
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
}
