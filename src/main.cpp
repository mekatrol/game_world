#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp> // ortho, translate, scale
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "renderer/sprite_renderer.hpp"
#include "util/sprite_sheet.hpp"

static void framebuffer_callback(GLFWwindow *, int w, int h)
{
    glViewport(0, 0, w, h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    GLFWwindow *window = glfwCreateWindow(
        mode->width,
        mode->height,
        "Factorio-like",
        monitor,
        nullptr);

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_callback);

    if (gladLoadGL((GLADloadfunc)glfwGetProcAddress) == 0)
    {
        // In a real project you should log an error and exit gracefully.
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create scope to cleanup resources before GL context closed
    {
        renderer::SpriteRenderer sprite_renderer;

        // The sheet is a single PNG containing many 128x128 sprites.
        // Your file: 2048x2560 => 16 columns x 20 rows => 320 sprites total.
        util::SpriteSheet belt_sheet(
            "assets/entity/transport-belt/transport-belt.png",
            128, // tile width (pixels)
            128  // tile height (pixels)
        );

        // How many tiles are in the sheet (calculated from image size / tile size).
        const int sprite_count = belt_sheet.sprite_count();

        // Which tile we are currently drawing.
        int current_sprite_index = 0;

        // Wall-clock timing (GLFW returns seconds since glfwInit()).
        // We advance the sprite about once per second.
        double last_advance_time = glfwGetTime();

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(window, &width, &height);

            // -----------------------------------------------------------------
            // Update animation (advance sprite once per second)
            // -----------------------------------------------------------------
            const double now = glfwGetTime();

            if (sprite_count > 0)
            {
                // If your frame rate stutters, more than 1 second could pass
                // between frames. This code "catches up" by advancing multiple
                // steps so the animation stays time-correct.
                const double seconds_per_frame = 0.1;
                const double elapsed = now - last_advance_time;

                if (elapsed >= seconds_per_frame)
                {
                    const int steps = static_cast<int>(elapsed / seconds_per_frame);
                    current_sprite_index = (current_sprite_index + steps) % sprite_count;
                    last_advance_time += static_cast<double>(steps) * seconds_per_frame;
                }
            }

            // -----------------------------------------------------------------
            // Build transforms (2.5D style: ortho projection + model transform)
            // -----------------------------------------------------------------

            // sprite placement + size (pixels)
            const float x = 100.0f;
            const float y = 80.0f;

            // Render at twice the sprite tile size:
            // source tile is 128x128, so draw 64x64 on screen.
            const float w = 64.0f;
            const float h = 64.0f;

            // Screen-space orthographic projection:
            // left=0, right=windowWidth
            // top=0, bottom=windowHeight (note: this flips Y so (0,0) is top-left)
            glm::mat4 proj = glm::ortho(
                0.0f, static_cast<float>(width),
                static_cast<float>(height), 0.0f,
                -1.0f, 1.0f);

            // Model matrix: move to (x,y) then scale a unit quad up to (w,h).
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(w, h, 1.0f));

            // Final transform used by the shader to position the quad.
            glm::mat4 mvp = proj * model;

            // -----------------------------------------------------------------
            // Draw
            // -----------------------------------------------------------------
            // Draw the current sprite tile from the sprite sheet.
            // This assumes your SpriteRenderer has a function like:
            //   draw_sheet_index(SpriteSheet&, mat4 mvp, int index)
            //
            // If your renderer uses a different signature, update this call.
            sprite_renderer.draw_sheet_index(belt_sheet, mvp, current_sprite_index);

            // Input
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            glfwSwapBuffers(window);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
