struct Splat {
    vec3 position;
    vec4 rotation;
    vec3 scale;
    vec4 color;
};

struct ScreenSplat {
    vec2 center;
    mat2 cov;
    vec4 color;
    vec2 offsets[4];
};

struct TileInfo {
    uint key_offset;
    uint key_count;
};
