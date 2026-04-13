#ifndef WATERENGINE_SHADER_PROGRAM_H
#define WATERENGINE_SHADER_PROGRAM_H
#include <string>
#include <stdexcept>

#include "glad/glad.h"
#include "glm/glm.hpp"

/** Runtime error related to OpenGL shader program initialization */
class shader_program_error : public std::runtime_error {
public:
    explicit shader_program_error(const std::string& msg) : std::runtime_error(msg) {}
};

enum class ShaderType {
    Vertex = GL_VERTEX_SHADER,
    Fragment = GL_FRAGMENT_SHADER,
    Compute = GL_COMPUTE_SHADER
};

/** Encapulates an OpenGL shader program */
class ShaderProgram {
public:
    explicit ShaderProgram(const std::vector<ShaderType>& shader_types, const std::vector<std::string>& shader_paths);

    /** Set this as the active shader program */
    void use() const;

    void set_int(const std::string& uniform, const int& value) const;
    void set_mat4(const std::string& uniform, const glm::mat4& value, const GLboolean transpose = GL_FALSE) const;
    void set_uint(const std::string& uniform, const unsigned int& value) const;
    void set_vec3(const std::string& uniform, const glm::vec3 value) const;
    void set_vec2(const std::string& uniform, const glm::vec2 value) const;

    /** Retrieve the shader program's id */
    unsigned int get_id() const;
private:
    unsigned int m_id;
};

#endif //WATERENGINE_SHADER_PROGRAM_H