#version 460
#define STB_INCLUDE_LINE_GLSL

#include "utils/gaussian.glsl"

uniform vec2 uResolution;

const float corr = 0.0;
const float var_x = 1.0;
const float var_y = 1.0;

out vec4 fragColor;

const mat2 cov = mat2(
    var_x, corr*sqrt(var_x)*sqrt(var_y),
    corr*sqrt(var_x)*sqrt(var_y), var_y
);


void main() {
    vec2 uv = (gl_FragCoord.xy / uResolution.xy) * 2.0 - 1.0;
    //fragColor = vec4(3.0 * standard_bivariate_pdf(uv));
    fragColor = vec4(2.0 * bivariate_pdf(uv * 5.0, cov));
}
