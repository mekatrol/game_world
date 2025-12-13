#pragma once

#include <glad/gl.h>

#include "renderer/shader.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace util
{
    class Texture;
    class SpriteSheet;
}

namespace renderer
{
    class SpriteRenderer
    {
    public:
        SpriteRenderer();
        ~SpriteRenderer();

        SpriteRenderer(const SpriteRenderer &) = delete;
        SpriteRenderer &operator=(const SpriteRenderer &) = delete;

        SpriteRenderer(SpriteRenderer &&) = delete;
        SpriteRenderer &operator=(SpriteRenderer &&) = delete;

        void draw(const util::Texture &texture, const glm::mat4 &mvp) const;

        void draw(
            const util::Texture &texture,
            const glm::mat4 &mvp,
            const glm::vec2 &uv_min,
            const glm::vec2 &uv_max);

        void draw_sheet_index(
            const util::SpriteSheet &sheet,
            const glm::mat4 &mvp,
            int index);

    private:
        void create_quad_buffers();
        void destroy_buffers();

    private:
        Shader m_shader{};
        GLuint m_vao{};
        GLuint m_vbo{};
    };
}
