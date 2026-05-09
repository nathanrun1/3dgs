/** Determines global histogram index corresponding to local index */
uint histogram_index(uint local_id, uint block_id, uint block_size, uint base_stride) {
    uint block_offset = block_id * base_stride * block_size;
    return (local_id + 1) * base_stride - 1 + block_offset;
}

/**
 * Retrieves histogram value at local index, or 0 if out of bounds
 * Assumes frequencies[] is a defined uniform corresponding to the histogram
 */
uint histogram_value_at(uint local_id, uint block_id, uint block_size, uint base_stride) {
    uint hist_index = histogram_index(local_id, block_id, block_size, base_stride);
    return hist_index < frequencies.length() ? frequencies[hist_index] : 0;
}

/** 
 * Writes given value from local index to global histogram
 * Assumes frequencies[] is a defined uniform corresponding to the histogram
 */
void write_histogram_value(uint local_id, uint block_id, uint block_size, uint base_stride, uint value) {
    uint hist_index = histogram_index(local_id, block_id, block_size, base_stride);
    if (hist_index < frequencies.length()) {
        frequencies[hist_index] = value;
    }
}
