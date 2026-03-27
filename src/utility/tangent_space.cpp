#include <iostream>

#include "tangent_space.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

struct TangentBasis {
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

/** Calculates tangent space basis for the given triangle defined by three vertices */
glm::mat2x3 _tangent_basis(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
    glm::vec3 edge1 = v1.position - v2.position;
    glm::vec3 edge2 = v1.position - v3.position;
    glm::vec2 delta_uv1 = v1.tex_coord - v2.tex_coord;
    glm::vec2 delta_uv2 = v1.tex_coord - v3.tex_coord;
    
    glm::mat2 delta_uv(delta_uv1, delta_uv2);
    glm::mat2x3 edges(edge1, edge2);


    glm::mat2 inv = glm::inverse(delta_uv);

    // [E1, E2]^T = [DUV1, DUV2]^T * [T, B]^T
    // -> [T, B]^T = [DUV1, DUV2]^T^-1 * [E1, E2]^T
    // That is, [T, B] transforms from tangent space to world space.
    glm::mat2x3 TB = edges * glm::inverse(delta_uv);
    return TB;
}

/**
 * Determines the handedness of the given tangent basis and normal, returning the sign that T x N should be multiplied
 * by to retrieve the bitangent
 */
float _get_bitangent_sign(const glm::mat2x3& tangent_basis, const glm::vec3& normal) {

    float result = glm::dot(glm::cross(tangent_basis[0], normal), tangent_basis[1]) > 0.0 ? 1.0 : -1.0;
    glm::vec3 cross = glm::cross(tangent_basis[0], normal);
    return result;
}

/** 
 * Updates the vertex's tangent (and invert_bitangent attribute) based on total amount of tangents calculated and most recently calculated tangent.
 * Uses the .w component of the newly calculated tangent.
 * 
 * @details accounts for shared vertices between triangles, interpolating the tangent calculations from each one.
 */
void _update_tangent(Vertex& v, unsigned int num_tangents, const glm::vec4& tangent) {
    if (num_tangents == 1) {
        v.tangent = tangent;
        return;
    }
    v.tangent = glm::vec4(glm::normalize(static_cast<float>(num_tangents) * xyz(v.tangent) + xyz(tangent)), tangent.w);
}

void calculate_tangents(const std::span<Vertex>& vertices, const std::span<unsigned int>& indices) {
    std::unordered_map<unsigned int, size_t> tangents_calculated{};
    for (int i = 0; i < indices.size(); i += 3) {
        glm::mat2x3 basis = _tangent_basis(
            vertices[indices[i]],
            vertices[indices[i+1]],
            vertices[indices[i+2]]
        );
        for (int j = i; j < i + 3; ++j) {
            Vertex& v = vertices[indices[j]];
            unsigned int num_tangents = ++tangents_calculated[indices[j]];
            glm::vec4 tangent = glm::vec4(basis[0], _get_bitangent_sign(basis, v.normal));  // w component contains handedness sign
            _update_tangent(v, num_tangents, tangent);
        }
    }
}
