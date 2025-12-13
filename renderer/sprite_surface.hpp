#pragma once

#include <glad/gl.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "renderer/shader.hpp"
#include "renderer/sprite_renderer.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "util/sprite_sheet.hpp"

namespace renderer
{
    class SpriteSurface
    {
    public:
        SpriteSurface(util::SpriteSheet *sheet, int offset, int count, double seconds_per_frame);
        ~SpriteSurface() {}

        void tick(double now, glm::mat4 &proj, renderer::SpriteRenderer *sprite_renderer, float scale);

        void set_position(float x, float y)
        {
            m_x = x;
            m_y = y;
        }

    private:
        util::SpriteSheet *m_sheet;
        int m_offset = 0;
        int m_count = 0;
        double m_last_advance_time = 0.0;
        float m_x = 0.0f;
        float m_y = 0.0f;
        int m_sprite_width = 0;
        int m_sprite_height = 0;
        int m_current_sprite_index = 0;
        // The time that a single frame lives for
        double m_seconds_per_frame = 0.05;
    };
}
