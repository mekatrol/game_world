#pragma once

#include <unordered_map>
#include <vector>

#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "shader.hpp"
#include "util/sprite_sheet.hpp"

namespace renderer
{
    struct SpriteInstance
    {
        glm::vec2 pos;
        glm::vec2 size;
        glm::vec4 uv;
        unsigned int frame_offset;
        unsigned int frame_index;
        unsigned int frame_count;
        double seconds_per_frame;
    };

    class SpriteRenderer
    {
    public:
        SpriteRenderer();
        ~SpriteRenderer();

        enum class BatchType
        {
            Sprite,
            Font
        };

        SpriteRenderer(const SpriteRenderer &) = delete;
        SpriteRenderer &operator=(const SpriteRenderer &) = delete;

        void begin_batch(const glm::mat4 &proj, BatchType type = BatchType::Sprite);

        void submit(util::SpriteSheet *sheet, const SpriteInstance &instance);
        void end_batch();

        void release() {
            destroy_buffers();
            m_sprite_shader.release();
            m_font_shader.release();
        }

    private:
        void create_buffers();
        void destroy_buffers();

    private:
        Shader m_sprite_shader;
        Shader m_font_shader;
        BatchType m_batch_type = BatchType::Sprite;

        GLuint m_vao{};
        GLuint m_quad_vbo{};
        GLuint m_instance_vbo{};

        glm::mat4 m_proj{1.0f};

        static constexpr size_t MaxInstances = 200000;

        std::unordered_map<util::SpriteSheet *, std::vector<SpriteInstance>> m_buckets;
    };
}
