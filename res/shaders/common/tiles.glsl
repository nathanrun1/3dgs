#define TILE_DIM 16

/** Calculates tile resolution (how many tiles per axis) */
uvec2 tile_res(uvec2 resolution) {
    return (resolution + TILE_DIM - 1) / TILE_DIM;
}

/** Calculates screen's total tile count */
uint tile_count(uvec2 resolution) {
    uvec2 t_res = tile_res(resolution);
    return t_res.x * t_res.y;
}

/** Converts tile ID to tile position */
uvec2 tile_pos_from_id(uint tile_id, uvec2 tile_res) {
    return uvec2(tile_id % tile_res.x, tile_id / tile_res.x);
}

/** Converts NDC to tile position */
uvec2 tile_pos_from_ndc(vec2 ndc, uvec2 tile_res) {
    return uvec2(((ndc + 1.0) / 2.0) * tile_res);  // floor(portion of screen * tile_res) for tile position
}

/** Converts tile position to tile ID */
uint tile_id_from_pos(uvec2 tile_pos, uint tile_res_x) {
    return tile_res_x * tile_pos.y + tile_pos.x;
}

/** Converts NDC to tile ID */
uint tile_id_from_ndc(vec2 ndc, uvec2 tile_res) {
    uvec2 tile_pos = uvec2(((ndc + 1.0) / 2.0) * tile_res);
    return tile_res.x * tile_pos.y + tile_pos.x;
}

/** Bottom left NDC of tile with given ID */
vec2 tile_offset(uint tile_id, uvec2 tile_res) {
    vec2 tile_pos = vec2(tile_pos_from_id(tile_id, tile_res));
    return (tile_pos / vec2(tile_res)) * 2.0 - 1.0;
}

/** Determines amount of tiles overlapped by the given ndc quad (aligned with axes) */
uint num_tiles_in_quad_ndc(vec2 tl, vec2 br, uvec2 tile_res) {
    uvec2 tile_pos_tl = tile_pos_from_ndc(tl, tile_res);
    uvec2 tile_pos_br = tile_pos_from_ndc(br, tile_res);

    return (tile_pos_br.x - tile_pos_tl.x + 1) * (tile_pos_tl.y - tile_pos_br.y + 1);  // W*H = Area of quad in tiles
}

/** Determines amount of tiles overlapped by the given tile-pos quad (aligned with axes) */
uint num_tiles_in_quad_pos(uvec2 tl, uvec2 br, uvec2 tile_res) {
    return (br.x - tl.x + 1) * (tl.y - br.y + 1);  // W*H = Area of quad in tiles
}

/** Determines whether tile of a given ID overlaps with the given quad in NDC */
//bool is_tile_in_quad(uint tile_id, uvec2 resolution, vec2 tl, vec2 tr, vec2 bl, vec2 br) {
//    uvec2 tile_pos_tl = tile_pos(tl, resolution);
//    uvec2 tile_pos_tr = tile_pos(tr, resolution);
//    uvec2 tile_pos_bl = tile_pos(bl, resolution);
//    uvec2 tile_pos_br = tile_pos(br, resolution);
//
//    uvec2 tile_pos = tile
//}
