#version 460
#define STB_INCLUDE_LINE_GLSL

#include "utils/gaussian.glsl"

struct ScreenSplat {
    vec2 center;
    mat2 cov;
    vec4 color;
    vec2 offsets[4];
};

layout (std430, binding = 0) readonly buffer ScreenSplats {
    ScreenSplat screen_splats[];
};

flat in uint splatIndex;
in vec2 vOffset;

out vec4 fragColor;

void main() {
    mat2 cov = screen_splats[splatIndex].cov;
    //fragColor = vec4(1.0) * bivariate_pdf(vOffset, cov);
    fragColor = screen_splats[splatIndex].color * bivariate_pdf(vOffset, cov);
}
