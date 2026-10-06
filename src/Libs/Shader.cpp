#include "Shader.h"
#include "ProjectPaths.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

std::string Shader::readFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open shader: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::compile(unsigned int type,
                             const std::string& source,
                             const std::string& path)
{
    const unsigned int shader = glCreateShader(type);
    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char infoLog[2048]{};
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        glDeleteShader(shader);

        throw std::runtime_error(
            "Shader compilation failed [" + path + "]\n" + infoLog
        );
    }

    return shader;
}

Shader::Shader(const std::string& vertexFile, const std::string& fragmentFile)
{
    const std::string vertexPath =
        std::string(SHADER_DIR) + "/" + vertexFile;
    const std::string fragmentPath =
        std::string(SHADER_DIR) + "/" + fragmentFile;

    const std::string vertexSource = readFile(vertexPath);
    const std::string fragmentSource = readFile(fragmentPath);

    const unsigned int vertexShader =
        compile(GL_VERTEX_SHADER, vertexSource, vertexPath);
    const unsigned int fragmentShader =
        compile(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);

    m_program = glCreateProgram();
    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);

    int success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);

    if (!success) {
        char infoLog[2048]{};
        glGetProgramInfoLog(m_program, sizeof(infoLog), nullptr, infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(m_program);
        m_program = 0;

        throw std::runtime_error(
            "Shader program linking failed:\n" + std::string(infoLog)
        );
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::cout << "Loaded shaders from:\n"
              << "  " << vertexPath << "\n"
              << "  " << fragmentPath << "\n";
}

Shader::~Shader()
{
    if (m_program) {
        glDeleteProgram(m_program);
    }
}

void Shader::use() const
{
    glUseProgram(m_program);
}

unsigned int Shader::id() const
{
    return m_program;
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_program, name.c_str()), value ? 1 : 0);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_program, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(m_program, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(
        glGetUniformLocation(m_program, name.c_str()),
        1,
        glm::value_ptr(value)
    );
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(
        glGetUniformLocation(m_program, name.c_str()),
        1,
        GL_FALSE,
        glm::value_ptr(value)
    );
}
