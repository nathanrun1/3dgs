#include "shader_program.h"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <sstream>

#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"

std::string parseShader(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw shader_program_error("Failed to open file: " + filepath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

ShaderProgram::ShaderProgram(const ShaderProgramInfo& info) {
    const std::string vertex_src = parseShader(info.vertexPath);
    const std::string fragment_src = parseShader(info.fragmentPath);
    const char* c_str;

    int success;
    std::string infoLog;
    infoLog.resize(512);

    // Vertex shader
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);  // Create shader object, and retrieves its ID
    c_str = vertex_src.c_str();
    glShaderSource(vertex_shader, 1, &c_str, nullptr);  // Provide shader object with source code
    glCompileShader(vertex_shader);  // Compile the shader object's underlying shader
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, infoLog.data());
        throw shader_program_error("Failed to compile vertex shader at " + info.vertexPath + ". Info log below.\n" + infoLog);
    }

    // Fragment shader
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    c_str = fragment_src.c_str();
    glShaderSource(fragment_shader, 1, &c_str, nullptr);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, infoLog.data());
        throw shader_program_error("Failed to compile fragment shader at " + info.fragmentPath + ". Info log below.\n" + infoLog);
    }

    // Full program
    m_id = glCreateProgram();  // Creates a program object, and retrieves its ID
    glAttachShader(m_id, vertex_shader);
    glAttachShader(m_id, fragment_shader);
    glLinkProgram(m_id);  // Link compiled shaders

    glGetProgramiv(m_id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_id, 512, nullptr, infoLog.data());
        throw shader_program_error("Shader program linking failed! Info log below.\n" + infoLog);
    }

    glDeleteShader(vertex_shader);  // Delete now-obsolete shader objects
    glDeleteShader(fragment_shader);
}

void ShaderProgram::use() const {
    glUseProgram(m_id);
}

void ShaderProgram::set_int(const std::string& uniform, const int &value) const {
    glUniform1i(glGetUniformLocation(m_id, uniform.c_str()), value);
}

void ShaderProgram::set_mat4(const std::string& uniform, const glm::mat4 &value, GLboolean transpose) const {
    glUniformMatrix4fv(glGetUniformLocation(m_id, uniform.c_str()), 1, transpose, glm::value_ptr(value));
}

void ShaderProgram::set_uint(const std::string& uniform, const unsigned int &value) const {
    glUniform1ui(glGetUniformLocation(m_id, uniform.c_str()), value);
}

void ShaderProgram::set_vec3(const std::string& uniform, const glm::vec3 value) const {
    glUniform3fv(glGetUniformLocation(m_id, uniform.c_str()), 1, glm::value_ptr(value));
}

void ShaderProgram::set_vec2(const std::string& uniform, const glm::vec2 value) const {
    glUniform2fv(glGetUniformLocation(m_id, uniform.c_str()), 1, glm::value_ptr(value));
}

unsigned int ShaderProgram::get_id() const {
    return m_id;
}

