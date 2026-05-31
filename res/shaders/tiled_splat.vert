#version 460

#include "common/tiles.glsl"

uniform uvec2 uResolution;

out vec2 vPos;
out uint vTileId;


vec2 corners[4] = {vec2(0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0)};

void main() {
    vec2 tile_dim_ndc = (2.0 * vec2(TILE_DIM)) / vec2(uResolution);
    vec2 offset = tile_offset(gl_InstanceID, uResolution);
    
    vPos = offset + corners[gl_VertexID] * tile_dim_ndc;
    gl_Position = vec4(offset + corners[gl_VertexID] * tile_dim_ndc, 0.0, 1.0);
}
