#define PI 3.14159265359

float gauss_pdf(float z2) {
    return (1 / sqrt(2*PI)) * exp(-0.5 * z2);
}

float standard_bivariate_pdf(vec2 z) {
    return gauss_pdf(z.x * z.x) * gauss_pdf(z.y * z.y);
}

float bivariate_pdf(vec2 x, mat2 cov) {
    mat2 inv_cov = inverse(cov);
    float z2 = dot(x, inverse(cov) * x);
    float pdf = (1 / (2*PI)) * exp(-0.5 * z2);
    return (1 / sqrt(determinant(cov))) * pdf;
}
