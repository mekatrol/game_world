#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/sprite_renderer.hpp"
#include "util/fps_counter.hpp"
#include "util/sprite_sheet.hpp"
#include "util/msdf_font.hpp"

static void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
    glViewport(0, 0, w, h);
}

int main(int argc, char **argv)
{
    FpsCounter fps_counter;

    if (!glfwInit())
    {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "Instanced Sprites (GL 3.3)", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {
        renderer::SpriteRenderer sprite_renderer_1;
        renderer::SpriteRenderer sprite_renderer_2;

        // Sheets
        util::SpriteSheet transport_belt_sheet("assets/entity/transport-belt/transport-belt.png", 128, 128, false);
        util::SpriteSheet worm_attack_sheet("assets/entity/worm/worm-attack-2.png", 1920 / 4, 1760 / 4, false);

        util::MsdfFont font;
        font.load("assets/fonts/font.json", "assets/fonts/font.png");
        font.sheet().texture().set_filtering(GL_LINEAR, GL_LINEAR);

        GLuint fontTexId = font.sheet().texture().id();

        glBindTexture(GL_TEXTURE_2D, fontTexId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Simulate many entities: 10k sprites in a grid.
        // Scale this up to 100k+ to test throughput (and add culling in real usage).
        const int sprite_count = 100000;

        std::vector<renderer::SpriteInstance> instances;
        instances.resize(sprite_count);

        for (int i = 0; i < sprite_count; ++i)
        {
            unsigned int frame_count = i % 2 == 0 ? 16 : 16;
            unsigned int frame_offset = i % 2 == 0 ? (i * frame_count) % (frame_count * frame_count) : 0;

            instances[i].frame_index = 0;
            instances[i].frame_offset = frame_offset;
            instances[i].frame_count = frame_count;
            instances[i].seconds_per_frame = 0.05;
        }

        // The previous sprite advance time
        double prev_advance_time = glfwGetTime();
        static double anim_time = 0.0;

        while (!glfwWindowShouldClose(window))
        {
            double now = glfwGetTime();
            fps_counter.tick(now);

            // The amount of time that has apssed since last frame
            const double elapsed = now - prev_advance_time;

            glfwPollEvents();

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            int w = 0, h = 0;
            glfwGetFramebufferSize(window, &w, &h);

            // Pixel-perfect 2D projection (top-left origin).
            const glm::mat4 proj = glm::ortho(0.0f, (float)w, (float)h, 0.0f);

            // Start sprite rendering
            sprite_renderer_1.begin_batch(&transport_belt_sheet, proj, renderer::SpriteRenderer::BatchType::Sprite);
            sprite_renderer_2.begin_batch(&worm_attack_sheet, proj, renderer::SpriteRenderer::BatchType::Sprite);

            // Submit per-instance data.
            // In a real engine, you’d iterate visible entities, choose their frame index,
            // compute UV, then submit.
            const int cols = 500;

            anim_time += elapsed;

            for (int i = 0; i < sprite_count; ++i)
            {
                const float x = (float)(i % cols) * 32.0f;
                const float y = (float)(i / cols) * 32.0f;

                auto &instance = instances[i];

                const int frame = static_cast<int>(anim_time / instance.seconds_per_frame) % instance.frame_count;
                instance.frame_index = instance.frame_offset + frame;

                if (i % 2 == 0)
                {
                    sprite_renderer_1.submit(renderer::SpriteInstance{
                        .pos = {x, y},
                        .size = {64.0f, 64.0f},
                        .uv = transport_belt_sheet.uv_rect_vec4(instance.frame_index),
                    });
                }
                else
                {

                    sprite_renderer_2.submit(renderer::SpriteInstance{
                        .pos = {x, y},
                        .size = {64.0f, 64.0f},
                        .uv = worm_attack_sheet.uv_rect_vec4(instance.frame_index),
                    });
                }
            }

            prev_advance_time = now;

            sprite_renderer_1.end_batch();
            sprite_renderer_2.end_batch();

            sprite_renderer_1.begin_batch(&font.sheet(), proj, renderer::SpriteRenderer::BatchType::Font);

            font.render_text(
                sprite_renderer_1,
                "FPS: " + std::to_string(fps_counter.fps),
                10.0f,
                10.0f,
                1.0f);

            sprite_renderer_1.end_batch();

            glfwSwapBuffers(window);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
