#pragma once

#include "texture.hpp"
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
        SpriteSheet(const std::string &path, int sprite_count_x, int sprite_count_y, bool flip);

        bool has_mask() const noexcept;
        bool has_shadow() const noexcept;

        bool load_from_file(const std::string &path, int sprite_count_x, int sprite_count_y, bool flip);

        bool load_night_overlays(const std::string &mask_path,
                                 const std::string &shadow_path,
                                 bool flip);

        const Texture &texture() const noexcept;
        const Texture &mask_texture() const noexcept;
        const Texture &shadow_texture() const noexcept;

        Texture &texture() noexcept;
        Texture &mask_texture() noexcept;
        Texture &shadow_texture() noexcept;

        int sprite_width() const noexcept;
        int sprite_height() const noexcept;

        int columns() const noexcept;
        int rows() const noexcept;
        int sprite_count() const noexcept;

        // Returns UV rectangle as vec4: (u0, v0, u1, v1) in normalized 0..1
        glm::vec4 uv_rect_vec4(int sprite_index) const;
        glm::vec4 uv_from_grid(int col, int row, int cols, int rows) const;

    private:
        bool validate() const;

    private:
        Texture m_texture{};
        Texture m_mask_texture{};
        Texture m_shadow_texture{};

        int m_sprite_width{};
        int m_sprite_height{};
    };
}
