#ifndef WATERENGINE_RENDERER_H
#define WATERENGINE_RENDERER_H
#include <map>
#include <memory>

#include "shader_program.h"
#include "assets/materials.h"
#include "assets/meshes.h"
#include "world/transform.h"

namespace Renderer {
    /** Initializes renderer with given shader program */
    void init(const std::string& program_id);

    /** Starts the frame draw */
    void begin_draw();

    /** Draws the given mesh */
    void draw_mesh(const Assets::Mesh& mesh, const Transform& transform, const Assets::Material& material);
    
    /** Draws the given points */
    void draw_points(const std::vector<glm::vec3>& points);

    /** Creates shader program from given shaders and adds it to available program list */
    void create_program(const std::string& program_id, const ShaderProgramInfo &program_info);

    /** Uses the shader program with the given id */
    void use_program(const std::string& program_id);
}

#endif //WATERENGINE_RENDERER_H


// TODO
// - Render a set of points
// - Retrieve a set of points from python that represents some gaussian splat defined by a covariance matrix