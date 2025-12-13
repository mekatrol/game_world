#pragma once

#include <glad/gl.h>

#include <string>

#include <glm/mat4x4.hpp>

namespace renderer
{
    class Shader
    {
    public:
        Shader() = default;
        Shader(const std::string &vertex_path, const std::string &fragment_path);
        ~Shader();

        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;

        Shader(Shader &&other) noexcept;
        Shader &operator=(Shader &&other) noexcept;

        void load_from_files(const std::string &vertex_path, const std::string &fragment_path);

        void use() const;

        [[nodiscard]] GLuint id() const noexcept { return m_program; }
        [[nodiscard]] bool valid() const noexcept { return m_program != 0; }

        void set_int(const char *name, int value) const;
        void set_mat4(const char *name, const glm::mat4 &value) const;

    private:
        static std::string read_file(const std::string &path);

        static GLuint compile_stage(GLenum type, const std::string &source, const std::string &debug_name);
        static GLuint link_program(GLuint vs, GLuint fs);

        void release();

    private:
        GLuint m_program{};
    };
}
