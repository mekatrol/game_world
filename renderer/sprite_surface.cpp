#include "renderer/sprite_surface.hpp"
#include "renderer/sprite_renderer.hpp"

#include <glm/gtc/matrix_transform.hpp> // ortho, translate, scale
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace renderer
{
    SpriteSurface::SpriteSurface(util::SpriteSheet *sheet, int offset, int count)
    {
        m_sheet = sheet;
        m_offset = offset;
        m_count = count;
        m_sprite_width = sheet->sprite_width();
        m_sprite_height = sheet->sprite_height();
    }

    void SpriteSurface::tick(double now, glm::mat4 &proj, renderer::SpriteRenderer *sprite_renderer, float scale)
    {
        // If your frame rate stutters, more than 1 second could pass
        // between frames. This code "catches up" by advancing multiple
        // steps so the animation stays time-correct.
        const double seconds_per_frame = 0.05;
        const double elapsed = now - m_last_advance_time;

        if (elapsed >= seconds_per_frame)
        {
            const int steps = static_cast<int>(elapsed / seconds_per_frame);
            m_current_sprite_index = (m_current_sprite_index + steps) % m_count + (m_offset * m_count);
            m_last_advance_time += static_cast<double>(steps) * seconds_per_frame;
        }

        // Model matrix: move to (x,y) then scale a unit quad up to (w,h).
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_x, m_y, 0.0f));
        model = glm::scale(model, glm::vec3(m_sprite_width, m_sprite_height, scale));

        // Final transform used by the shader to position the quad.
        glm::mat4 mvp = proj * model;

        // Render current sprite
        sprite_renderer->draw_sheet_index(m_sheet, mvp, m_current_sprite_index);
    }
}