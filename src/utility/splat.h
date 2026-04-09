#ifndef WATERENGINE_SPLAT_H
#define WATERENGINE_SPLAT_H
#include <string>
#include <vector>

#include <glm/glm.hpp>

struct Splat {
    glm::vec3 position;
    glm::vec4 rotation;
    glm::vec3 scale;
    glm::vec4 color;
};

/**
 * Loads a list of splats from the given .ply file
 */
std::vector<Splat> load_splats(const std::string& ply_file);

#endif //WATERENGINE_SPLAT_H