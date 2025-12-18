#include "sprite_sheet.hpp"

namespace util
{
    SpriteSheet::SpriteSheet(const std::string &path, int sprite_count_x, int sprite_count_y, bool flip)
    {
        load_from_file(path, sprite_count_x, sprite_count_y, flip);
    }

    bool SpriteSheet::has_mask() const noexcept { return m_mask_sheet.texture.is_valid(); }
    bool SpriteSheet::has_shadow() const noexcept { return m_shadow_sheet.texture.is_valid(); }

    bool SpriteSheet::load_from_file(const std::string &path, int sprite_count_x, int sprite_count_y, bool flip)
    {
        bool success = m_base_sheet.texture.load_from_file(path, flip);

        if (!success)
        {
            return false;
        }

        m_base_sheet.sprite_width = m_base_sheet.texture.width() / sprite_count_x;
        m_base_sheet.sprite_height = m_base_sheet.texture.height() / sprite_count_y;

        return validate();
    }

    bool SpriteSheet::load_night_overlays(const std::string &mask_path,
                                          const std::string &shadow_path,
                                          bool flip)
    {
        // Preserved original logic exactly (even though it looks inverted).
        if (!mask_path.empty() && m_mask_sheet.texture.load_from_file(mask_path, flip))
        {
            return false;
        }

        return mask_path.empty() || m_shadow_sheet.texture.load_from_file(shadow_path, flip);
    }

    const Sprite &SpriteSheet::base_sheet() const noexcept { return m_base_sheet; }
    const Sprite &SpriteSheet::mask_sheet() const noexcept { return m_mask_sheet; }
    const Sprite &SpriteSheet::shadow_sheet() const noexcept { return m_shadow_sheet; }

    Sprite &SpriteSheet::base_sheet() noexcept { return m_base_sheet; }
    Sprite &SpriteSheet::mask_sheet() noexcept { return m_mask_sheet; }
    Sprite &SpriteSheet::shadow_sheet() noexcept { return m_shadow_sheet; }

    int SpriteSheet::columns() const noexcept
    {
        return (m_base_sheet.sprite_width > 0) ? (m_base_sheet.texture.width() / m_base_sheet.sprite_width) : 0;
    }

    int SpriteSheet::rows() const noexcept
    {
        return (m_base_sheet.sprite_height > 0) ? (m_base_sheet.texture.height() / m_base_sheet.sprite_height) : 0;
    }

    int SpriteSheet::sprite_count() const noexcept
    {
        return columns() * rows();
    }

    glm::vec4 SpriteSheet::uv_rect_vec4(int sprite_index) const
    {
        const int cols = columns();

        if (cols <= 0 || m_base_sheet.sprite_width <= 0 || m_base_sheet.sprite_height <= 0)
        {
            return {0.0f, 0.0f, 1.0f, 1.0f};
        }

        const int x = sprite_index % cols;
        const int y = sprite_index / cols;

        const float tex_w = (float)m_base_sheet.texture.width();
        const float tex_h = (float)m_base_sheet.texture.height();

        const float u0 = (x * m_base_sheet.sprite_width) / tex_w;
        const float v0 = (y * m_base_sheet.sprite_height) / tex_h;
        const float u1 = ((x + 1) * m_base_sheet.sprite_width) / tex_w;
        const float v1 = ((y + 1) * m_base_sheet.sprite_height) / tex_h;

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
        if (m_base_sheet.texture.id() == 0 || m_base_sheet.texture.width() <= 0 || m_base_sheet.texture.height() <= 0)
        {
            return false;
        }
        if (m_base_sheet.sprite_width <= 0 || m_base_sheet.sprite_height <= 0)
        {
            return false;
        }
        if ((m_base_sheet.texture.width() % m_base_sheet.sprite_width) != 0)
        {
            return false;
        }
        if ((m_base_sheet.texture.height() % m_base_sheet.sprite_height) != 0)
        {
            return false;
        }
        return true;
    }
}
