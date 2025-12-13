#include "renderer/sprite_renderer.hpp"

#include "util/sprite_sheet.hpp"
#include "util/texture.hpp"

#include <stdexcept>

namespace renderer
{
    SpriteRenderer::SpriteRenderer()
        : m_shader("assets/shaders/sprite.vert", "assets/shaders/sprite.frag")
    {
        create_quad_buffers();

        m_shader.use();
        m_shader.set_int("uTexture", 0);
        glUseProgram(0);
    }

    SpriteRenderer::~SpriteRenderer()
    {
        destroy_buffers();
        // m_shader destructor deletes the GL program (must happen before GLFW context is destroyed)
    }

    void SpriteRenderer::create_quad_buffers()
    {
        // Allocate VBO large enough for dynamic UV updates (6 verts * 4 floats).
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

        // layout(location=0) vec2 aPos
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

        // layout(location=1) vec2 aUV
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void SpriteRenderer::destroy_buffers()
    {
        if (m_vbo != 0)
        {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }

        if (m_vao != 0)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
    }

    void SpriteRenderer::draw(const util::Texture &texture, const glm::mat4 &mvp) const
    {
        // Full texture UVs
        const auto uv_min = glm::vec2(0.0f, 0.0f);
        const auto uv_max = glm::vec2(1.0f, 1.0f);

        // const method calls non-const draw not allowed, so duplicate minimal path:
        m_shader.use();
        m_shader.set_mat4("uMVP", mvp);

        texture.bind(0);
        m_shader.set_int("uTexture", 0);

        const float verts[] = {
            // x, y,   u, v
            0.0f,
            0.0f,
            uv_min.x,
            uv_min.y,
            1.0f,
            0.0f,
            uv_max.x,
            uv_min.y,
            1.0f,
            1.0f,
            uv_max.x,
            uv_max.y,

            0.0f,
            0.0f,
            uv_min.x,
            uv_min.y,
            1.0f,
            1.0f,
            uv_max.x,
            uv_max.y,
            0.0f,
            1.0f,
            uv_min.x,
            uv_max.y,
        };

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        texture.unbind();
        glUseProgram(0);
    }

    void SpriteRenderer::draw(
        const util::Texture &texture,
        const glm::mat4 &mvp,
        const glm::vec2 &uv_min,
        const glm::vec2 &uv_max)
    {
        m_shader.use();
        m_shader.set_mat4("uMVP", mvp);

        texture.bind(0);
        m_shader.set_int("uTexture", 0);

        const float verts[] = {
            // x, y,   u, v
            0.0f,
            0.0f,
            uv_min.x,
            uv_min.y,
            1.0f,
            0.0f,
            uv_max.x,
            uv_min.y,
            1.0f,
            1.0f,
            uv_max.x,
            uv_max.y,

            0.0f,
            0.0f,
            uv_min.x,
            uv_min.y,
            1.0f,
            1.0f,
            uv_max.x,
            uv_max.y,
            0.0f,
            1.0f,
            uv_min.x,
            uv_max.y,
        };

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);
        texture.unbind();
        glUseProgram(0);
    }

    void SpriteRenderer::draw_sheet_index(const util::SpriteSheet *sheet, const glm::mat4 &mvp, int index)
    {
        const auto [uv0, uv1] = sheet->uv_rect(index);
        draw(sheet->texture(), mvp, uv0, uv1);
    }
}
