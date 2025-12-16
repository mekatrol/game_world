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

void draw_text(
    renderer::SpriteRenderer &renderer,
    util::SpriteSheet &font,
    const std::string &text,
    float x,
    float y,
    float scale = 1.0f)
{
    constexpr int FONT_COLS = 18;
    constexpr int FONT_ROWS = 7;

    float cursor_x = x;

    for (char c : text)
    {
        int ascii = (int)c;
        if (ascii < 32 || ascii > 126)
        {
            cursor_x += font.sprite_width() * scale;
            continue;
        }

        int index = ascii - 32;
        int col = index % FONT_COLS;
        int row = index / FONT_COLS;

        renderer.submit(renderer::SpriteInstance{
            .pos = {cursor_x, y},
            .size = {
                font.sprite_width() * scale,
                font.sprite_height() * scale},
            .uv = font.uv_from_grid(col, row, FONT_COLS, FONT_ROWS)});

        cursor_x += font.sprite_width() * scale;
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
        util::SpriteSheet sheet("assets/entity/transport-belt/transport-belt.png", 128, 128);
        util::SpriteSheet font_sheet("assets/fonts/font.png", 5, 7);

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

            sprite_renderer.begin_batch(&sheet, proj);

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

            draw_text(
                sprite_renderer,
                font_sheet,
                "FPS: " + std::to_string(fps_counter.fps),
                5.0f, // left
                5.0f, // top
                2.0f  // scale
            );

            sprite_renderer.end_batch();

            glfwSwapBuffers(window);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
