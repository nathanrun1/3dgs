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

uniform vec3 uBackgroundColor;

void main() {
    mat2 cov = screen_splats[splatIndex].cov;
    //fragColor = vec4(1.0) * bivariate_pdf(vOffset, cov);
    float pdf = unnormalized_bivariate_pdf(vOffset, cov);
    //fragColor = vec4(mix(uBackgroundColor, screen_splats[splatIndex].color.rgb, pdf), pdf);
    fragColor = mix(vec4(uBackgroundColor, 0.0), vec4(screen_splats[splatIndex].color.rgb, 1.0), pdf);
}
