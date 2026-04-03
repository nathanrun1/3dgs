#include "shader_program.h"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <sstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#define STB_INCLUDE_IMPLEMENTATION
#define STB_INCLUDE_LINE_GLSL
#include <stb_include.h>

#include "utility/config/config.h"

std::string _parse_shader(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw shader_program_error("Failed to open file: " + filepath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/** Compiles the given source code into the given GL shader object. Returns shader object's ID. */
GLuint _compile_shader(const GLuint shader, const std::string& src_path) {
    const char* include_path = Config::get_value(Config::ConfigGroup::Shaders, "paths", "include").c_str();
    
    std::string error;
    error.resize(512);
    const char* src = stb_include_file(
        const_cast<char*>(src_path.c_str()), 
        nullptr, 
        const_cast<char*>(include_path), 
        error.data()
    );
    if (!src) {
        throw shader_program_error("Preprocessing of shader " + src_path + " failed. Error below.\n" + error);
    }
    
    glShaderSource(shader, 1, &src, nullptr);  // Provide shader object with source code
    glCompileShader(shader);  // Compile the shader object's underlying shader
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, error.data());
        throw shader_program_error("Failed to compile vertex shader at " + src_path + ". Info log below.\n" + error);
    }
    
    return shader;
}

ShaderProgram::ShaderProgram(const ShaderProgramInfo& info) {

    GLuint vertex_shader = _compile_shader(glCreateShader(GL_VERTEX_SHADER), info.vertexPath);
    GLuint fragment_shader = _compile_shader(glCreateShader(GL_FRAGMENT_SHADER), info.fragmentPath);

    // Full program
    m_id = glCreateProgram();  // Creates a program object, and retrieves its ID
    glAttachShader(m_id, vertex_shader);
    glAttachShader(m_id, fragment_shader);
    glLinkProgram(m_id);  // Link compiled shaders
    
    int success;
    std::string infoLog;
    infoLog.resize(512);
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

