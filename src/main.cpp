#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp> // ortho, translate, scale
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "renderer/sprite_renderer.hpp"
#include "util/texture.hpp"

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
        // handle error
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create scope to cleanup resource before GL context closed
    {

        renderer::SpriteRenderer sprite_renderer;
        util::Texture belt("assets/entity/transport-belt/transport-belt.png");

        float time = 0.0f;

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            time += 0.016f;

            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(window, &width, &height);

            // sprite placement + size (pixels)
            const float x = 100.0f;
            const float y = 80.0f;
            const float w = 32.0f;
            const float h = 32.0f;

            glm::mat4 proj = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            model = glm::scale(model, glm::vec3(w, h, 1.0f));

            glm::mat4 mvp = proj * model;
            sprite_renderer.draw(belt, mvp);

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
