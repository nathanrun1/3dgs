#ifndef WATERENGINE_U_BLOCKS_H
#define WATERENGINE_U_BLOCKS_H
#include "assets/splat.h"
#include "glm/vec3.hpp"
#include "world/lighting.h"

// Structs that include padding to align with either the std140 UBO layout or std430 SSBO layout, depending on 
//   whichever it's used for.
namespace Renderer {
    constexpr size_t MAX_LIGHTS = 8;

    struct alignas(16) UBLight {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 color;
        alignas(4)  unsigned int type;
        alignas(4)  float intensity;
        
        UBLight() = default;

        explicit UBLight(const World::Light& light)
            : position{light.position}
            , color{light.color}
            , type{static_cast<unsigned int>(light.type)}
            , intensity{light.intensity}
        {}
    };

    struct alignas(16) UBLighting {
        alignas(4)  unsigned int num_lights;
        alignas(16) UBLight lights[MAX_LIGHTS];
    };
    
    struct alignas(16) UBMaterial {
        alignas(4)  unsigned int albedo_id;
        alignas(4)  unsigned int roughness_id;
        alignas(4)  unsigned int metallic_id;
        alignas(4)  unsigned int normal_id;
        alignas(4)  unsigned int displacement_id;
        
        alignas(16) glm::vec3 albedo_scale;
        alignas(4)  float roughness_scale;
        alignas(4)  float metallic_scale;
        alignas(4)  float displacement_scale;

        alignas(4)  unsigned int flags;
        
        UBMaterial() = default;
        
        explicit UBMaterial(const Assets::Material& material)
            : albedo_id{material.albedo_id}
            , roughness_id{material.roughness_id}
            , metallic_id{material.metallic_id}
            , normal_id{material.normal_id}
            , displacement_id{material.displacement_id}
            , albedo_scale{material.albedo_scale}
            , roughness_scale{material.roughness_scale}
            , metallic_scale{material.metallic_scale}
            , displacement_scale{material.displacement_scale}
            , flags{static_cast<unsigned int>(material.flags)}
        {}
    };
    
    struct alignas(16) SSBSplat {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec4 rotation;
        alignas(16) glm::vec3 scale;
        alignas(16) glm::vec4 color;
        
        SSBSplat() = default;
        
        explicit SSBSplat(const Assets::Splat& splat)
            : position{splat.position}
            , rotation{splat.rotation}
            , scale{splat.scale}
            , color{splat.color}
        {}
    };
    
    struct alignas(16) SSBScreenSplat {
        alignas(8)  glm::vec2 center;
        alignas(8)  glm::mat2 cov;
        alignas(16) glm::vec4 color;
        alignas(8)  glm::vec2 offsets[4];
    };
}

#endif //WATERENGINE_U_BLOCKS_H