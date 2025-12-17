#pragma once

#include <vector>
#include <glad/gl.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "shader.hpp"
#include "util/sprite_sheet.hpp"

namespace renderer
{
    // Per-instance payload.
    // Keep this small and tightly packed.
    struct SpriteInstance
    {
        glm::vec2 pos;             // top-left in pixels (because our ortho uses top-left origin)
        glm::vec2 size;            // size in pixels
        glm::vec4 uv;              // (u0, v0, u1, v1) in normalized 0..1
        unsigned int frame_offset; // The offset of the first sprite in the sprite sheet
        unsigned int frame_index;  // Current sprite index
        unsigned int frame_count;  // The total number of sprites for this animation
        double seconds_per_frame;  // The time that a single frame lives for
    };

    // Instanced sprite renderer:
    // - static unit quad (6 verts)
    // - instance VBO with SpriteInstance data
    // - 1 draw call per batch (per sprite sheet)
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

        void begin_batch(
            util::SpriteSheet *sheet,
            const glm::mat4 &proj,
            BatchType type = BatchType::Sprite);
        void submit(const SpriteInstance &instance);
        void end_batch();

        // Optional: reserve capacity to avoid re-allocations.
        void reserve(size_t n) { m_instances.reserve(n); }

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

        util::SpriteSheet *m_sheet{};
        glm::mat4 m_proj{1.0f};

        std::vector<SpriteInstance> m_instances;

        // Max instances per batch buffer allocation.
        static constexpr size_t MaxInstances = 200000;
    };
}
