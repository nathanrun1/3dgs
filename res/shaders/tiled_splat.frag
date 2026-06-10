#version 460
#define STB_INCLUDE_LINE_GLSL

#include "utils/gaussian.glsl"
#include "common/structs.glsl"


layout (std430, binding = 0) readonly buffer ScreenSplats {
    ScreenSplat screen_splats[];
};

layout (std430, binding = 6) readonly buffer SortedSplatIndices {
    uint sorted_indices[];
};

uniform vec3 uBackgroundColor;

in vec2 vPos;
flat in uint vKeyBegin;
flat in uint vKeyEnd;
out vec4 fragColor;

vec3 alpha_blend(vec3 dst, vec3 src, float alpha) {
    return (1 - alpha) * dst + alpha * src;
}

void main() {
    vec4 final_color = vec4(uBackgroundColor, 1.0);
    
    // Manually alpha blend all splats within the tile
    for (uint i = vKeyBegin; i < vKeyEnd; ++i) {
        uint sid = sorted_indices[i];
        mat2 cov = screen_splats[sid].cov;
        float pdf = unnormalized_bivariate_pdf(vPos - screen_splats[sid].center, cov);
        vec3 splat_color = screen_splats[sid].color.rgb;
        final_color = vec4(alpha_blend(final_color.rgb, splat_color.rgb, pdf), 1.0);
    }
    
    fragColor = final_color;
}
