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

    renderer::SpriteRenderer sprite_renderer;

    // Sheets
    util::SpriteSheet transport_belt_sheet("assets/entity/transport-belt/transport-belt.png", 16, 20, false);
    util::SpriteSheet worm_attack_sheet("assets/entity/worm/worm-attack-2.png", 4, 4, false);

    util::MsdfFont font;
    font.load("assets/fonts/font.json", "assets/fonts/font.png");
    font.sheet().texture().set_filtering(GL_LINEAR, GL_LINEAR);

    const int sprite_count = 100000;

    std::vector<renderer::SpriteInstance> instances(sprite_count);

    for (int i = 0; i < sprite_count; ++i)
    {
        instances[i].frame_offset = 0;
        instances[i].frame_index = 0;
        instances[i].frame_count = 16;
        instances[i].seconds_per_frame = 0.05;
    }

    double prev_advance_time = glfwGetTime();
    double anim_time = 0.0;

    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        fps_counter.tick(now);

        const double elapsed = now - prev_advance_time;
        prev_advance_time = now;

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);

        const glm::mat4 proj = glm::ortho(0.0f, (float)w, (float)h, 0.0f);

        sprite_renderer.begin_batch(proj, renderer::SpriteRenderer::BatchType::Sprite);

        const int cols = 500;
        anim_time += elapsed;

        for (int i = 0; i < sprite_count; ++i)
        {
            const float x = (float)(i % cols) * 32.0f;
            const float y = (float)(i / cols) * 32.0f;

            auto &inst = instances[i];
            const int frame = static_cast<int>(anim_time / inst.seconds_per_frame) % inst.frame_count;
            inst.frame_index = inst.frame_offset + frame;

            renderer::SpriteInstance draw_instance{
                .pos = {x, y},
                .size = {64.0f, 64.0f},
            };

            if ((i & 1) == 0)
            {
                draw_instance.uv = transport_belt_sheet.uv_rect_vec4(inst.frame_index);
                sprite_renderer.submit(&transport_belt_sheet, draw_instance);
            }
            else
            {
                draw_instance.uv = worm_attack_sheet.uv_rect_vec4(inst.frame_index);
                sprite_renderer.submit(&worm_attack_sheet, draw_instance);
            }
        }

        sprite_renderer.end_batch();

        // Font pass
        sprite_renderer.begin_batch(proj, renderer::SpriteRenderer::BatchType::Font);

        font.render_text(
            sprite_renderer,
            &font.sheet(),
            "FPS: " + std::to_string(fps_counter.fps),
            10.0f,
            10.0f,
            1.0f);

        sprite_renderer.end_batch();

        glfwSwapBuffers(window);
    }

    sprite_renderer.release();
    transport_belt_sheet.texture().release();
    worm_attack_sheet.texture().release();
    font.sheet().texture().release();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
