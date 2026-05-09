#version 460

#define TILE_DIM 16

uniform uvec2 uResolution;

out vec2 vPos;
out uint vTileId;


vec2 corners[4] = {vec2(0.0), vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0)};

/** Bottom left NDC of tile with given ID */
vec2 tile_offset(uint tile_id) {
    uvec2 tile_res = (uResolution + TILE_DIM - 1) / TILE_DIM;
    vec2 tile_pos = vec2(tile_id % tile_res.x, tile_id / tile_res.x);
    return (tile_pos / vec2(tile_res)) * 2.0 - 1.0;
}

void main() {
    vec2 tile_dim_ndc = vec2(TILE_DIM) / vec2(uResolution);
    vec2 offset = tile_offset(gl_InstanceID);
    
    vPos = offset + corners[gl_VertexID] * tile_dim_ndc;
    gl_Position = vec4(offset + corners[gl_VertexID] * tile_dim_ndc, 0.0, 1.0);
}
