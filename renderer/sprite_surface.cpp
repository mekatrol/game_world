#include "renderer/sprite_surface.hpp"
#include "renderer/sprite_renderer.hpp"

#include <glm/gtc/matrix_transform.hpp> // ortho, translate, scale
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace renderer
{
    SpriteSurface::SpriteSurface(util::SpriteSheet *sheet, int offset, int count, double seconds_per_frame)
    {
        m_sheet = sheet;
        m_offset = offset;
        m_count = count;
        m_sprite_width = sheet->sprite_width();
        m_sprite_height = sheet->sprite_height();
        m_seconds_per_frame = seconds_per_frame;
    }

    void SpriteSurface::tick(double now, glm::mat4 &proj, renderer::SpriteRenderer *sprite_renderer, float scale)
    {
        // The amount of time that has apssed since last frame
        const double elapsed = now - m_last_advance_time;

        // Are we ready for next frame?
        if (elapsed >= m_seconds_per_frame)
        {
            // If there has been a large delay (elapsed exceeds multiple frame times) then we may need
            // to step multiple frames
            const int steps = static_cast<int>(elapsed / m_seconds_per_frame);

            // Index next sprite
            m_current_sprite_index = m_offset + ((m_current_sprite_index - m_offset + steps) % m_count);

            // Keey track of 'now' in frame time
            m_last_advance_time += static_cast<double>(steps) * m_seconds_per_frame;
        }

        // Model matrix: move to (x,y) then scale a unit quad up to (w,h).
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(m_x, m_y, 0.0f));
        model = glm::scale(model, glm::vec3(m_sprite_width, m_sprite_height, scale));

        // Final transform used by the shader to position the quad.
        glm::mat4 model_projection = proj * model;

        // Render current sprite
        sprite_renderer->draw_sheet_index(m_sheet, model_projection, m_current_sprite_index);
    }
}