#include "sprite_renderer.hpp"

#include <cstddef> // offsetof
#include <stdexcept>

namespace renderer
{
    SpriteRenderer::SpriteRenderer()
        : m_shader("assets/shaders/sprite.vert", "assets/shaders/sprite.frag")
    {
        create_buffers();

        m_shader.use();
        m_shader.set_int("uTexture", 0);
        glUseProgram(0);
    }

    SpriteRenderer::~SpriteRenderer()
    {
        destroy_buffers();
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
            0.0f, 1.0f
        };

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        // Quad vertices (static)
        glGenBuffers(1, &m_quad_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

        // aPos at location=0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

        // Instance buffer (dynamic)
        glGenBuffers(1, &m_instance_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        glBufferData(GL_ARRAY_BUFFER, MaxInstances * sizeof(SpriteInstance), nullptr, GL_STREAM_DRAW);

        // Instance attributes:
        // layout(location=1) vec2 iPos
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance), (void*)offsetof(SpriteInstance, pos));
        glVertexAttribDivisor(1, 1);

        // layout(location=2) vec2 iSize
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance), (void*)offsetof(SpriteInstance, size));
        glVertexAttribDivisor(2, 1);

        // layout(location=3) vec4 iUV
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance), (void*)offsetof(SpriteInstance, uv));
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

    void SpriteRenderer::begin_batch(util::SpriteSheet* sheet, const glm::mat4& proj)
    {
        m_sheet = sheet;
        m_proj = proj;
        m_instances.clear();
    }

    void SpriteRenderer::submit(const SpriteInstance& instance)
    {
        m_instances.push_back(instance);
    }

    void SpriteRenderer::end_batch()
    {
        if (!m_sheet || m_instances.empty())
        {
            return;
        }

        // If you exceed MaxInstances, you can:
        // - split into multiple draws, OR
        // - increase MaxInstances
        if (m_instances.size() > MaxInstances)
        {
            throw std::runtime_error("SpriteRenderer: batch exceeded MaxInstances");
        }

        m_shader.use();
        m_shader.set_mat4("uProj", m_proj);

        // Bind texture once per batch (sheet)
        m_sheet->texture().bind(0);

        glBindVertexArray(m_vao);

        // Upload instance data (one upload per batch)
        glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_instances.size() * sizeof(SpriteInstance), m_instances.data());

        // One draw call for all instances
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)m_instances.size());

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
    }
}
