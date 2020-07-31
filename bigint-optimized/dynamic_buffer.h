#ifndef BIGINT_DYNAMIC_BUFFER_H
#define BIGINT_DYNAMIC_BUFFER_H

#include <cstddef>
#include <cstdint>
#include <vector>

struct dynamic_buffer {
    std::vector<uint32_t> data;

    dynamic_buffer();

    explicit dynamic_buffer(size_t size);

    explicit dynamic_buffer(std::vector<uint32_t> const& vec);

    dynamic_buffer(dynamic_buffer const&) = delete;
    dynamic_buffer& operator=(dynamic_buffer const&) = delete;

    bool unique();

    dynamic_buffer* makeCopy();

    void reduceCounter();
private:
    size_t ref_counter;
};

#endif //BIGINT_DYNAMIC_BUFFER_H
