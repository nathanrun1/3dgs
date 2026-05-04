/** Extract near (.x) and far (.y) plane values from projection matrix */
vec2 near_far(mat4 proj) {
    float A = proj[2][2];
    float B = proj[3][2];

    float n = (2.0f * B) / (A - 1.0f);
    float f = B / (A + 1.0f);

    return vec2(n, f);
}