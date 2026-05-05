#version 460

struct ScreenSplat {
    vec2 center;
    mat2 cov;
    vec4 color;
    vec2 offsets[4];
};

layout (std430, binding = 0) readonly buffer ScreenSplats {
    ScreenSplat screen_splats[];
};

layout (std430, binding = 6) readonly buffer SortedSplatIndices {
    uint sorted_indices[];
};

vec2 my_offsets[4] = {vec2(1.0), vec2(-1.0, 1.0), vec2(-1.0), vec2(1.0, -1.0)};

flat out uint splatIndex;
out vec2 vOffset;

void main() {
    splatIndex = sorted_indices[gl_InstanceID];
    vOffset = screen_splats[splatIndex].offsets[gl_VertexID];

    gl_Position = vec4(screen_splats[splatIndex].center + vOffset, 0.0, 1.0);
}
