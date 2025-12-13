#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp> // ortho, translate, scale
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "renderer/sprite_renderer.hpp"
#include "renderer/sprite_surface.hpp"
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

        renderer::SpriteSurface sprite_surface_1(&belt_sheet, 0, 16);
        renderer::SpriteSurface sprite_surface_2(&belt_sheet, 1, 16);

        sprite_surface_2.set_position(100, 300);

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

            glm::mat4 proj = glm::ortho(
                0.0f, static_cast<float>(width),
                static_cast<float>(height), 0.0f,
                -1.0f, 1.0f);

            sprite_surface_1.tick(now, proj, &sprite_renderer, 0.5f);
            sprite_surface_2.tick(now, proj, &sprite_renderer, 0.5f);

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
