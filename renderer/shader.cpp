#include "renderer/shader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <glm/gtc/type_ptr.hpp>

namespace renderer
{
    Shader::Shader(const std::string &vertex_path, const std::string &fragment_path)
    {
        load_from_files(vertex_path, fragment_path);
    }

    Shader::~Shader()
    {
        release();
    }

    Shader::Shader(Shader &&other) noexcept
        : m_program(std::exchange(other.m_program, 0))
    {
    }

    Shader &Shader::operator=(Shader &&other) noexcept
    {
        if (this != &other)
        {
            release();
            m_program = std::exchange(other.m_program, 0);
        }
        return *this;
    }

    void Shader::load_from_files(const std::string &vertex_path, const std::string &fragment_path)
    {
        release();

        const auto vs_src = read_file(vertex_path);
        const auto fs_src = read_file(fragment_path);

        const auto vs = compile_stage(GL_VERTEX_SHADER, vs_src, vertex_path);
        const auto fs = compile_stage(GL_FRAGMENT_SHADER, fs_src, fragment_path);

        m_program = link_program(vs, fs);

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    void Shader::use() const
    {
        glUseProgram(m_program);
    }

    void Shader::set_int(const char *name, int value) const
    {
        const auto loc = glGetUniformLocation(m_program, name);
        if (loc >= 0)
        {
            glUniform1i(loc, value);
        }
    }

    void Shader::set_mat4(const char *name, const glm::mat4 &value) const
    {
        const auto loc = glGetUniformLocation(m_program, name);
        if (loc >= 0)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
        }
    }

    std::string Shader::read_file(const std::string &path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to open file: " + path);
        }

        std::ostringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }

    GLuint Shader::compile_stage(GLenum type, const std::string &source, const std::string &debug_name)
    {
        const auto shader = glCreateShader(type);

        const char *src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint ok = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);

            std::string log;
            log.resize(static_cast<size_t>(len > 1 ? len : 1));

            GLsizei written = 0;
            glGetShaderInfoLog(shader, len, &written, log.data());

            glDeleteShader(shader);

            throw std::runtime_error("Shader compile failed (" + debug_name + "):\n" + log);
        }

        return shader;
    }

    GLuint Shader::link_program(GLuint vs, GLuint fs)
    {
        const auto prog = glCreateProgram();
        glAttachShader(prog, vs);
        glAttachShader(prog, fs);
        glLinkProgram(prog);

        GLint ok = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok)
        {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);

            std::string log;
            log.resize(static_cast<size_t>(len > 1 ? len : 1));

            GLsizei written = 0;
            glGetProgramInfoLog(prog, len, &written, log.data());

            glDeleteProgram(prog);

            throw std::runtime_error("Program link failed:\n" + log);
        }

        return prog;
    }

    void Shader::release()
    {
        if (m_program != 0)
        {
            glDeleteProgram(m_program);
            m_program = 0;
        }
    }
}
