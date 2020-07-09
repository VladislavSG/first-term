#include "dynamic_buffer.h"

dynamic_buffer::dynamic_buffer() : ref_counter(1) {}

dynamic_buffer::dynamic_buffer(size_t size) :
        data(size),
        ref_counter(1) {}

dynamic_buffer::dynamic_buffer(std::vector<uint32_t>& vec) :
        data(vec),
        ref_counter(1) {}

bool dynamic_buffer::unique() {
    return ref_counter == 1;
}

dynamic_buffer *dynamic_buffer::makeCopy() {
    ++ref_counter;
    return this;
}

void dynamic_buffer::reduceCounter() {
    if (--ref_counter == 0) {
        delete this;
    }
}
