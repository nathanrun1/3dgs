#ifndef WATERENGINE_SPLAT_H
#define WATERENGINE_SPLAT_H
#include <string>
#include <vector>
#include <span>

#include <glm/glm.hpp>

namespace Assets {
    struct Splat {
        glm::vec3 position;
        glm::vec4 rotation;
        glm::vec3 scale;
        glm::vec4 color;
    };

    /** Loads a list of splats from the given .ply file to be used in the scene */
    void load_splats(const std::string& ply_file);

    /** Retrieves the list of splats to be rendered in the scene */
    std::span<const Splat> get_splats();

    /** Retrieves the list of splats, sorted ascending by viewspace z, to be rendered in the scene */
    std::span<const Splat> get_splats_sorted(const glm::mat4& view_mat);
}



#endif //WATERENGINE_SPLAT_H