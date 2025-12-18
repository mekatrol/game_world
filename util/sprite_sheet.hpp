#pragma once

#include "texture.hpp"
#include <glm/vec4.hpp>
#include <string>

namespace util
{
    struct Sprite
    {
        Texture texture{};

        int sprite_width{};
        int sprite_height{};
    };

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

        const Sprite &base_sprite() const noexcept;
        const Sprite &mask_sprite() const noexcept;
        const Sprite &shadow_sprite() const noexcept;

        Sprite &base_sprite() noexcept;
        Sprite &mask_sprite() noexcept;
        Sprite &shadow_sprite() noexcept;

        int columns() const noexcept;
        int rows() const noexcept;
        int sprite_count() const noexcept;

        // Returns UV rectangle as vec4: (u0, v0, u1, v1) in normalized 0..1
        glm::vec4 uv_rect_vec4(int sprite_index) const;
        glm::vec4 uv_from_grid(int col, int row, int cols, int rows) const;

    private:
        bool validate() const;

    private:
        Sprite m_base_sprite;
        Sprite m_shadow_sprite;
        Sprite m_mask_sprite;
    };
}
