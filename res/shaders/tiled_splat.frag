#version 460
#define STB_INCLUDE_LINE_GLSL

#include "utils/gaussian.glsl"

struct ScreenSplat {
    vec2 center;
    mat2 cov;
    vec4 color;
    vec2 offsets[4];
};

struct TileInfo {
    uint key_offset;
    uint key_count;
};

layout (std430, binding = 0) readonly buffer ScreenSplats {
    ScreenSplat screen_splats[];
};

layout (std430, binding = 6) readonly buffer SortedSplatIndices {
    uint sorted_indices[];
};

layout (std430, binding = 8) readonly buffer TileKeyInfo {
    TileInfo tile_info[];
};

uniform vec3 uBackgroundColor;

in vec2 vPos;
in uint vTileId;
out vec4 fragColor;

vec3 alpha_blend(vec3 dst, vec3 src, float alpha) {
    return (1 - alpha) * dst + alpha * src;
}

void main() {
    vec4 final_color = uBackgroundColor;
    
    TileInfo tinfo = tile_info[vTileId];
    
    // Manually alpha blend all splats within the tile
    for (uint i = tinfo.key_offset; i < tinfo.key_offset + tinfo.key_count; ++i) {
        uint sid = sorted_indices[i];
        mat2 cov = screen_splats[sid].cov;
        float pdf = unnormalized_bivariate_pdf(vPos - screen_splats[sid].center, cov);
        vec3 splat_color = screen_splats[sid].color.rgb;
        final_color = vec4(alpha_blend(final_color.rgb, splat_color.rgb, pdf), 1.0);
    }
    
    fragColor = final_color;
}
