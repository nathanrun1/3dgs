#version 460

uniform vec2 uResolution;

#define PI 3.14159265359

const float corr = -0.9;
const float var_x = 1.0;
const float var_y = 1.0;

const mat2 cov = mat2(
    var_x, corr*var_x*var_y,
    corr*var_x*var_y, var_y
);

// Calculates standard gaussian pdf for given z^2 value
float gauss_pdf(float z2) {
    return (1 / sqrt(2*PI)) * exp(-0.5 * z2);
}

float standard_bivariate_pdf(vec2 z) {
    return gauss_pdf(z.x * z.x) * gauss_pdf(z.y * z.y);
}

float bivariate_pdf(vec2 x) {
    mat2 inv_cov = inverse(cov);
    float z2 = dot(x, inverse(cov) * x);
    float pdf = (1 / (2*PI)) * exp(-0.5 * z2);
    return (1 / sqrt(determinant(inv_cov))) * pdf;
}

void main() {
    vec2 uv = (gl_FragCoord.xy / uResolution.xy) * 2.0 - 1.0;
    //gl_FragColor = vec4(3.0 * standard_bivariate_pdf(uv));
    gl_FragColor = vec4(5.0 * bivariate_pdf(uv * 5.0));
}
