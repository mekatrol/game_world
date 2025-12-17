#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <unordered_map>
#include <memory>

#include "renderer/sprite_renderer.hpp"
#include "util/fps_counter.hpp"
#include "util/sprite_sheet.hpp"
#include "util/msdf_font.hpp"
#include "util/animation_library.hpp"

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

    const int sprite_count = 100000;

    // Load animation definitions from json
    const auto animations = util::load_animation_library("assets/animations");

    std::unordered_map<std::string, std::unique_ptr<util::SpriteSheet>> sheets_by_key;
    for (const auto &[key, def] : animations)
    {
        sheets_by_key.emplace(
            key,
            std::make_unique<util::SpriteSheet>(
                def.asset_file,
                def.sprite_count_x,
                def.sprite_count_y,
                false));
    }

    // Build a flat list of (sheet, sequence) pairs to cycle through
    struct RuntimeAnim
    {
        const util::AnimationDef *def;
        const util::FrameSequence *sequence;
    };

    std::vector<RuntimeAnim> runtime_anims;
    for (const auto &[_, def] : animations)
    {
        for (const auto &[_, seq] : def.sequences)
        {
            runtime_anims.push_back({&def, &seq});
        }
    }

    std::vector<renderer::SpriteInstance> instances(sprite_count);

    for (int i = 0; i < sprite_count; ++i)
    {
        const auto &ra = runtime_anims[static_cast<size_t>(i) % runtime_anims.size()];

        instances[i].frame_index = 0;
        instances[i].frame_sequence = std::span<const unsigned int>(ra.sequence->frames);
        instances[i].seconds_per_frame = ra.sequence->seconds_per_frame;
    }

    renderer::SpriteRenderer sprite_renderer;

    util::MsdfFont font;
    font.load("assets/fonts/font.json", "assets/fonts/font.png");
    font.sheet().texture().set_filtering(GL_LINEAR, GL_LINEAR);

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

            const auto &ra = runtime_anims[static_cast<size_t>(i) % runtime_anims.size()];
            auto *sheet = sheets_by_key.at(ra.def->key).get();

            draw_instance.uv = sheet->uv_rect_vec4(instance.frame_index);
            sprite_renderer.submit(sheet, draw_instance);
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

    for (auto &[key, sheet] : sheets_by_key)
    {
        sheet->texture().release();
    }
    
    font.sheet().texture().release();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
