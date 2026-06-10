#version 460

#include "common/tiles.glsl"

uniform uvec2 uResolution;
uniform uint uNumSplats;

out vec2 vPos;
flat out uint vKeyBegin;
flat out uint vKeyEnd;

layout (std430, binding = 8) buffer TileHistogram {
    uint tile_histogram[];
};

vec2 corners[4] = {vec2(0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0)};

void main() {
    uint tile_id = gl_InstanceID;
    vKeyBegin = tile_histogram[tile_id];
    vKeyEnd = tile_id == tile_count(uResolution) - 1 ? uNumSplats                     // Last tile: Total # of keys
                                                     : tile_histogram[tile_id + 1]; // Otherwise: Next tile's offset

    vec2 tile_dim_ndc = (2.0 * vec2(TILE_DIM)) / vec2(uResolution);
    vec2 offset = tile_offset(tile_id, uResolution);
    
    vPos = offset + corners[gl_VertexID] * tile_dim_ndc;
    gl_Position = vec4(offset + corners[gl_VertexID] * tile_dim_ndc, 0.0, 1.0);
}
