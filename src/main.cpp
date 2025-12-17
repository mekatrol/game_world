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

static std::vector<unsigned int> offset_frames(const std::vector<unsigned int> &frames, unsigned int offset)
{
    std::vector<unsigned int> new_frames;
    new_frames.reserve(frames.size());

    for (auto frame : frames)
    {
        new_frames.push_back(frame + offset);
    }

    return new_frames;
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
    util::SpriteSheet fast_transport_belt_sheet("assets/entity/fast-transport-belt/fast-transport-belt.png", 32, 20, false);
    util::SpriteSheet worm_attack_sheet("assets/entity/worm/worm-attack-2.png", 4, 4, false);

    util::MsdfFont font;
    font.load("assets/fonts/font.json", "assets/fonts/font.png");
    font.sheet().texture().set_filtering(GL_LINEAR, GL_LINEAR);

    const int sprite_count = 100000;

    std::vector<unsigned int> transport_1_belt_frames = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    std::span<const unsigned int> transport_belt_frame_sequence_1{transport_1_belt_frames};

    std::vector<unsigned int> transport_2_belt_frames = offset_frames(transport_1_belt_frames, 16);
    std::span<const unsigned int> transport_belt_frame_sequence_2{transport_2_belt_frames};

    std::vector<unsigned int> transport_3_belt_frames = offset_frames(transport_1_belt_frames, 128);
    std::span<const unsigned int> transport_belt_frame_sequence_3{transport_3_belt_frames};

    std::vector<unsigned int> worm_attack_frames = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::span<const unsigned int> worm_attack_frame_sequence{worm_attack_frames};

    std::vector<renderer::SpriteInstance> instances(sprite_count);

    const unsigned int SEQUENCE_COUNT = 4;

    for (int i = 0; i < sprite_count; ++i)
    {
        instances[i].frame_index = 0;

        switch (i % SEQUENCE_COUNT)
        {
        case 0:
            instances[i].frame_sequence = transport_belt_frame_sequence_1;
            instances[i].seconds_per_frame = 0.05;
            break;
        case 1:
            instances[i].frame_sequence = transport_belt_frame_sequence_2;
            instances[i].seconds_per_frame = 0.05;
            break;
        case 2:
            instances[i].frame_sequence = transport_belt_frame_sequence_3;
            instances[i].seconds_per_frame = 0.03;
            break;
        case 3:
            instances[i].frame_sequence = worm_attack_frame_sequence;
            instances[i].seconds_per_frame = 0.1;
            break;
        default:
            throw std::runtime_error("Invalid index");
            break;
        }
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

            auto &instance = instances[i];
            const unsigned int frame_sequence_index = static_cast<int>(anim_time / instance.seconds_per_frame) % instance.frame_sequence.size();
            instance.frame_index = instance.frame_sequence[frame_sequence_index];

            renderer::SpriteInstance draw_instance{
                .pos = {x, y},
                .size = {64.0f, 64.0f},
            };

            switch (i % SEQUENCE_COUNT)
            {
            case 0:
            case 1:
                draw_instance.uv = transport_belt_sheet.uv_rect_vec4(instance.frame_index);
                sprite_renderer.submit(&transport_belt_sheet, draw_instance);
                break;
            case 2:
                draw_instance.uv = fast_transport_belt_sheet.uv_rect_vec4(instance.frame_index);
                sprite_renderer.submit(&fast_transport_belt_sheet, draw_instance);
                break;
            case 3:
                draw_instance.uv = worm_attack_sheet.uv_rect_vec4(instance.frame_index);
                sprite_renderer.submit(&worm_attack_sheet, draw_instance);
                break;
            default:
                throw std::runtime_error("Invalid index");
                break;
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
    fast_transport_belt_sheet.texture().release();
    worm_attack_sheet.texture().release();
    font.sheet().texture().release();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
