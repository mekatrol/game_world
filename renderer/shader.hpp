// shader.hpp
#pragma once

#include <string>

#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace renderer
{
    // Minimal shader helper:
    // - loads vertex/fragment shader from files
    // - compiles + links
    // - sets uniforms
    class Shader
    {
    public:
        Shader(const std::string &vertex_path, const std::string &fragment_path);
        ~Shader();

        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;

        Shader(Shader &&other) noexcept;
        Shader &operator=(Shader &&other) noexcept;

        void use() const;
        GLuint id() const noexcept { return m_program; }

        void set_int(const char *name, int value) const;
        void set_float(const std::string &name, float v) const;
        void set_vec4(const std::string &name, const glm::vec4 &v) const;
        void set_mat4(const char *name, const glm::mat4 &value) const;

    private:
        GLuint m_program{};

        static std::string read_file(const std::string &path);
        static GLuint compile_stage(GLenum type, const std::string &source, const std::string &debug_name);
        static GLuint link_program(GLuint vs, GLuint fs);

        void release();

        GLint get_uniform_location(const std::string &name) const;
    };
}
