/** Determines global histogram index corresponding to local index */
uint histogram_index(uint local_id, uint block_id, uint block_size, uint base_stride) {
    uint block_offset = block_id * base_stride * block_size;
    return (local_id + 1) * base_stride - 1 + block_offset;
}
