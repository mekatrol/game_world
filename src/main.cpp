// main.cpp
//
// Notes on performance:
// - All JSON and sprite sheet creation happens once at startup.
// - The hot loop does NOT do any unordered_map lookups or string hashing.
// - The hot loop avoids per-sprite division/modulo for animation timing.
// - Sprite positions are precomputed once (no i%cols / i/cols each frame).
// - Optional: sprites are submitted grouped-by-sheet to minimize texture/state changes.

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "renderer/sprite_renderer.hpp"
#include "util/animation_library.hpp"
#include "util/fps_counter.hpp"
#include "util/msdf_font.hpp"
#include "util/sprite_sheet.hpp"

static void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
    glViewport(0, 0, w, h);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    FpsCounter fps_counter;

    // -----------------------------
    // Window / GL init
    // -----------------------------
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

    // V-sync
    glfwSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // -----------------------------
    // Scene configuration
    // -----------------------------
    const int sprite_count = 100000;
    const int cols = 500;

    // -----------------------------
    // Load animation definitions (JSON) once
    // -----------------------------
    // Expected schema (per animation json):
    // {
    //   "key": "...",
    //   "assetFile": "...",
    //   "spriteCountX": N,
    //   "spriteCountY": M,
    //   "frameSequences": {
    //     "sequenceName": { "frames":[...], "secondsPerFrame": 0.05 },
    //     ...
    //   }
    // }
    const auto animations = util::load_animation_library("assets/animations");

    // -----------------------------
    // Create one SpriteSheet per animation key
    // -----------------------------
    // Stored in a map only for lifetime management and release().
    // We will NOT use this map in the hot loop.
    std::unordered_map<std::string, std::unique_ptr<util::SpriteSheet>> sheets_by_key;
    sheets_by_key.reserve(animations.size());

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

    // -----------------------------
    // Flatten animations into a list of runtime options
    // -----------------------------
    // This resolves the sheet pointer ONCE and keeps a pointer to the sequence ONCE.
    // No string hashing, no map access in the hot loop.
    struct RuntimeAnim
    {
        util::SpriteSheet *sheet = nullptr;
        const util::FrameSequence *sequence = nullptr;
    };

    std::vector<RuntimeAnim> runtime_anims;
    runtime_anims.reserve(64);

    for (const auto &[key, def] : animations)
    {
        // Lookup ONCE here (startup time).
        auto *sheet = sheets_by_key.at(key).get();

        for (const auto &[seq_name, seq] : def.sequences)
        {
            (void)seq_name; // sequence names are arbitrary; not needed at runtime here
            runtime_anims.push_back(RuntimeAnim{sheet, &seq});
        }
    }

    // Must have at least 1 animation sequence loaded.
    if (runtime_anims.empty())
    {
        // Nothing to display; clean shutdown.
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // -----------------------------
    // Allocate sprite instances + cached per-sprite data
    // -----------------------------
    std::vector<renderer::SpriteInstance> instances(sprite_count);

    // Cache the sheet pointer per sprite, so the draw loop never touches a map.
    std::vector<util::SpriteSheet *> instance_sheets(sprite_count, nullptr);

    // Cache frame sequence pointers/lengths for ultra-cheap access.
    std::vector<const unsigned int *> frames_ptr(sprite_count, nullptr);
    std::vector<uint32_t> frames_len(sprite_count, 0);

    // Cache per-sprite timing (float is plenty here).
    std::vector<float> seconds_per_frame(sprite_count, 0.1f);

    // Per-sprite animation state (no division/modulo needed per frame).
    std::vector<float> anim_accum(sprite_count, 0.0f);
    std::vector<uint32_t> frame_cursor(sprite_count, 0);

    // Precompute positions once (avoid i%cols and i/cols in the hot loop).
    std::vector<glm::vec2> positions(sprite_count);
    positions.reserve(sprite_count);

    for (int i = 0; i < sprite_count; ++i)
    {
        const auto &ra = runtime_anims[static_cast<size_t>(i) % runtime_anims.size()];

        // Assign the sprite's chosen sheet + sequence once.
        instance_sheets[i] = ra.sheet;

        frames_ptr[i] = ra.sequence->frames.data();
        frames_len[i] = static_cast<uint32_t>(ra.sequence->frames.size());
        seconds_per_frame[i] = static_cast<float>(ra.sequence->seconds_per_frame);

        // Initialize frame index to first frame (safe even if size==0, but size should be >0).
        frame_cursor[i] = 0;
        anim_accum[i] = 0.0f;

        instances[i].frame_index = (frames_len[i] > 0) ? frames_ptr[i][0] : 0;
        instances[i].frame_sequence = std::span<const unsigned int>(ra.sequence->frames);
        instances[i].seconds_per_frame = ra.sequence->seconds_per_frame;

        // Precompute sprite position.
        const float x = static_cast<float>(i % cols) * 32.0f;
        const float y = static_cast<float>(i / cols) * 32.0f;
        positions[i] = {x, y};
    }

    // -----------------------------
    // Optional: group indices by sheet to minimize texture/state changes
    // -----------------------------
    // If your SpriteRenderer breaks batches on texture changes, submitting mixed sheets
    // will hurt. Grouping ensures we submit contiguous runs by sheet.
    std::unordered_map<util::SpriteSheet *, std::vector<int>> sheet_to_indices;
    sheet_to_indices.reserve(sheets_by_key.size());

    for (int i = 0; i < sprite_count; ++i)
    {
        sheet_to_indices[instance_sheets[i]].push_back(i);
    }

    // -----------------------------
    // Renderer + font
    // -----------------------------
    renderer::SpriteRenderer sprite_renderer;

    util::MsdfFont font;
    font.load("assets/fonts/font.json", "assets/fonts/font.png");
    font.sheet().texture().set_filtering(GL_LINEAR, GL_LINEAR);

    // Timing
    double prev_time = glfwGetTime();

    // -----------------------------
    // Main loop
    // -----------------------------
    while (!glfwWindowShouldClose(window))
    {
        const double now = glfwGetTime();
        fps_counter.tick(now);

        const double elapsed = now - prev_time;
        prev_time = now;

        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int w = 0, h = 0;
        glfwGetFramebufferSize(window, &w, &h);

        const glm::mat4 proj = glm::ortho(0.0f, static_cast<float>(w), static_cast<float>(h), 0.0f);

        // -----------------------------
        // Sprite pass
        // -----------------------------
        sprite_renderer.begin_batch(proj, renderer::SpriteRenderer::BatchType::Sprite);

        const float dt = static_cast<float>(elapsed);

        // Render grouped-by-sheet for fewer texture switches (often improves FPS).
        for (auto &[sheet, idxs] : sheet_to_indices)
        {
            // Submit all sprites that use this sheet.
            for (int idx : idxs)
            {
                // Advance animation using accumulator stepping:
                // - No division
                // - No modulo (wrap is a single compare)
                anim_accum[idx] += dt;

                const float spf = seconds_per_frame[idx];

                // Fast path: step at most one frame per tick (good for stable frame times).
                if (anim_accum[idx] >= spf)
                {
                    anim_accum[idx] -= spf;

                    uint32_t c = frame_cursor[idx] + 1;
                    if (c >= frames_len[idx])
                    {
                        c = 0;
                    }
                    frame_cursor[idx] = c;
                }

                const unsigned int frame = frames_ptr[idx][frame_cursor[idx]];
                instances[idx].frame_index = frame;

                // Build draw instance (pos from precomputed array).
                const glm::vec2 p = positions[idx];

                renderer::SpriteInstance draw_instance{
                    .pos = {p.x, p.y},
                    .size = {64.0f, 64.0f},
                };

                // Compute UV rect for current frame and submit.
                draw_instance.uv = sheet->uv_rect_vec4(frame);
                sprite_renderer.submit(sheet, draw_instance);
            }
        }

        sprite_renderer.end_batch();

        // -----------------------------
        // Font pass
        // -----------------------------
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

    // -----------------------------
    // Cleanup / release
    // -----------------------------
    sprite_renderer.release();

    // Release all textures created for sheets loaded from JSON.
    for (auto &[key, sheet] : sheets_by_key)
    {
        (void)key;
        sheet->texture().release();
    }

    // Release font texture.
    font.sheet().texture().release();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
