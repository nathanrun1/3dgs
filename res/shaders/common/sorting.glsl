#define BITS_PER_UINT 32

uint histogram_index(uint block_id, uint digit, uint num_blocks) {
    return digit * num_blocks + block_id;
}

uint key_index(uint block_id, uint offset, uint block_size) {
    return block_id * block_size + offset;
}

/** Retrieves digit value of a given key, assumes digit size <= uint size and uint size % digit size = 0 */
uint digit_value(uvec2 key, uint digit_size, uint digit_id) {
    uint bit_offset = digit_id * digit_size;
    uint word = bit_offset >= BITS_PER_UINT ? key.x : key.y;
    uint local_shift = bit_offset % BITS_PER_UINT;
    uint mask = (1u << digit_size) - 1u;
    return (word >> local_shift) & mask;
}
