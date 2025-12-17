// OpenGL 3.3 instanced sprite renderer example.
// Renders many sprites from a sprite sheet with ONE draw call per sheet.
//
// Expected assets:
//   assets/sprites.png   (a sprite sheet image)
// Expected shaders (copied at project root):
//   sprite.vert
//   sprite.frag
//
// This example intentionally keeps animation logic on the CPU:
// - choose current frame index per entity
// - compute uv rect
// - submit as per-instance data

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer/sprite_renderer.hpp"
#include "util/sprite_sheet.hpp"
#include "util/msdf_font.hpp"

extern int generate_atlas(int argc, char **argv);

struct FpsCounter
{
    double last_time = 0.0;
    int frames = 0;
    int fps = 0;

    void tick(double now)
    {
        frames++;
        if (now - last_time >= 1.0)
        {
            fps = frames;
            frames = 0;
            last_time = now;
        }
    }
};

static void framebuffer_size_callback(GLFWwindow *, int w, int h)
{
    glViewport(0, 0, w, h);
}

void draw_text_msdf(
    renderer::SpriteRenderer &renderer,
    const util::MsdfFont &font,
    const std::string &text,
    float x,
    float y, // top-left anchor (same “feel” as your current function)
    float scale = 1.0f)
{
    float cursor_x = x;

    // Treat y as top-left; convert to a baseline using the font's measured line height.
    const float baseline_y = y + font.line_height() * scale;

    for (unsigned char c : text)
    {
        // Basic ASCII set your atlas_generator emits (32..126). :contentReference[oaicite:4]{index=4}
        if (c < 32 || c > 126)
        {
            cursor_x += font.line_height() * 0.5f * scale;
            continue;
        }

        const util::MsdfGlyph *g = font.glyph((int)c);
        if (!g)
            continue;

        // Position quad using bearings (baseline-aligned)
        const float gx = cursor_x + g->bearingX * scale;
        const float gy = baseline_y - g->bearingY * scale;

        renderer.submit(renderer::SpriteInstance{
            .pos = {gx, gy},
            .size = {g->w * scale, g->h * scale},
            .uv = g->uv});

        cursor_x += g->advance * scale;
    }
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
        renderer::SpriteRenderer sprite_renderer;

        // Example sprite sheet: adjust path + tile size to match your image.
        util::SpriteSheet sheet("assets/entity/transport-belt/transport-belt.png", 128, 128, false);

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

        while (!glfwWindowShouldClose(window))
        {
            double now = glfwGetTime();
            fps_counter.tick(now);

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

            sprite_renderer.begin_batch(&sheet, proj, renderer::SpriteRenderer::BatchType::Sprite);

            // Submit per-instance data.
            // In a real engine, you’d iterate visible entities, choose their frame index,
            // compute UV, then submit.
            const int cols = 500;
            for (int i = 0; i < sprite_count; ++i)
            {
                const float x = (float)(i % cols) * 32.0f;
                const float y = (float)(i / cols) * 32.0f;

                const int frame = i % sheet.sprite_count();

                sprite_renderer.submit(renderer::SpriteInstance{
                    .pos = {x, y},
                    .size = {64.0f, 64.0f},
                    .uv = sheet.uv_rect_vec4(frame),
                });
            }

            sprite_renderer.end_batch();

            sprite_renderer.begin_batch(&font.sheet(), proj, renderer::SpriteRenderer::BatchType::Font);

            draw_text_msdf(
                sprite_renderer,
                font,
                "FPS: " + std::to_string(fps_counter.fps),
                1.0f,
                1.0f,
                1.0f);
                
            sprite_renderer.end_batch();

            glfwSwapBuffers(window);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
