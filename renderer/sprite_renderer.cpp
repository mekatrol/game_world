#include "sprite_renderer.hpp"

#include <algorithm> // std::min
#include <cstddef>   // offsetof
#include <stdexcept>

namespace renderer
{
    SpriteRenderer::SpriteRenderer()
        : m_sprite_shader("assets/shaders/sprite.vert", "assets/shaders/sprite.frag"), m_font_shader("assets/shaders/sprite.vert", "assets/shaders/font.frag")
    {
        create_buffers();

        m_sprite_shader.use();
        m_sprite_shader.set_int("u_texture", 0);

        m_font_shader.use();
        m_font_shader.set_int("u_texture", 0);

        glUseProgram(0);
    }

    SpriteRenderer::~SpriteRenderer()
    {
        release();
    }

    void SpriteRenderer::begin_batch(const glm::mat4 &proj, BatchType type)
    {
        m_proj = proj;
        m_batch_type = type;
        m_buckets.clear();
    }

    void SpriteRenderer::submit(util::SpriteSheet *sheet, const SpriteInstance &instance)
    {
        if (!sheet)
        {
            return;
        }

        if (m_buckets.bucket_count() >= MaxInstances)
        {
            end_batch();
            begin_batch(m_proj, m_batch_type);
        }

        m_buckets[sheet].push_back(instance);
    }

    void SpriteRenderer::end_batch()
    {
        if (m_buckets.empty())
        {
            return;
        }

        Shader &shader = (m_batch_type == BatchType::Font)
                             ? m_font_shader
                             : m_sprite_shader;

        shader.use();
        shader.set_mat4("u_proj", m_proj);

        if (m_batch_type == BatchType::Font)
        {
            shader.set_vec4("u_color", {1.0f, 1.0f, 1.0f, 1.0f});
        }

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

        for (auto &[sheet, instances] : m_buckets)
        {
            if (!sheet || instances.empty())
            {
                continue;
            }

            // --------------------
            // 1) Base sprite (always)
            // --------------------
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            shader.set_vec4("u_color", {1.0f, 1.0f, 1.0f, 1.0f});
            sheet->base_sheet().texture.bind(0);

            std::size_t offset = 0;
            while (offset < instances.size())
            {
                const std::size_t count =
                    std::min<std::size_t>(MaxInstances, instances.size() - offset);

                glBufferSubData(
                    GL_ARRAY_BUFFER,
                    0,
                    count * sizeof(SpriteInstance),
                    instances.data() + offset);

                glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)count);
                offset += count;
            }

            // --------------------
            // 2) Shadow pass (optional)
            // --------------------
            if (m_batch_type == BatchType::Sprite && sheet->has_shadow())
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                shader.set_vec4("u_color", {0.0f, 0.0f, 0.0f, 0.6f});
                sheet->shadow_sheet().texture.bind(0);

                std::size_t offset = 0;
                while (offset < instances.size())
                {
                    const std::size_t count =
                        std::min<std::size_t>(MaxInstances, instances.size() - offset);

                    glBufferSubData(
                        GL_ARRAY_BUFFER,
                        0,
                        count * sizeof(SpriteInstance),
                        instances.data() + offset);

                    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)count);
                    offset += count;
                }
            }
            
            // --------------------
            // 3) Mask pass (optional, multiply)
            // --------------------
            if (m_batch_type == BatchType::Sprite && sheet->has_mask())
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_DST_COLOR, GL_ZERO); // multiply

                shader.set_vec4("u_color", {1.0f, 1.0f, 1.0f, 1.0f});
                sheet->mask_sheet().texture.bind(0);

                offset = 0;
                while (offset < instances.size())
                {
                    const std::size_t count =
                        std::min<std::size_t>(MaxInstances, instances.size() - offset);

                    glBufferSubData(
                        GL_ARRAY_BUFFER,
                        0,
                        count * sizeof(SpriteInstance),
                        instances.data() + offset);

                    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)count);
                    offset += count;
                }

                // Restore default blend
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    }

    void SpriteRenderer::create_buffers()
    {
        // Unit quad (two triangles) in local space: [0..1] x [0..1]
        const float quad[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,

            0.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f};

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        // Quad vertices (static)
        glGenBuffers(1, &m_quad_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

        // aPos at location=0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

        // Instance buffer (dynamic)
        glGenBuffers(1, &m_instance_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        glBufferData(GL_ARRAY_BUFFER, MaxInstances * sizeof(SpriteInstance), nullptr, GL_STREAM_DRAW);

        // layout(location=1) vec2 i_pos
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance), (void *)offsetof(SpriteInstance, pos));
        glVertexAttribDivisor(1, 1);

        // layout(location=2) vec2 i_size
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance), (void *)offsetof(SpriteInstance, size));
        glVertexAttribDivisor(2, 1);

        // layout(location=3) vec4 i_uv
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance), (void *)offsetof(SpriteInstance, uv));
        glVertexAttribDivisor(3, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void SpriteRenderer::destroy_buffers()
    {
        if (m_instance_vbo)
        {
            glDeleteBuffers(1, &m_instance_vbo);
            m_instance_vbo = 0;
        }
        if (m_quad_vbo)
        {
            glDeleteBuffers(1, &m_quad_vbo);
            m_quad_vbo = 0;
        }
        if (m_vao)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
    }

}
