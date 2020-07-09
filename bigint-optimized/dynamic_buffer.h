#ifndef BIGINT_DYNAMIC_BUFFER_H
#define BIGINT_DYNAMIC_BUFFER_H

#include <cstddef>
#include <vector>

template <typename T>
struct dynamic_buffer {
    std::vector<T> data;

    dynamic_buffer() : ref_counter(1) {};

    dynamic_buffer(size_t size) :
        data(size),
        ref_counter(1) {};

    dynamic_buffer(std::vector<T> vec) :
            data(vec),
            ref_counter(1) {};

    dynamic_buffer(dynamic_buffer const&) = delete;
    dynamic_buffer& operator=(dynamic_buffer const&) = delete;

    bool unique() {
        return ref_counter == 1;
    }

    dynamic_buffer* makeCopy() {
        ++ref_counter;
        return this;
    }

    void unshare() {
        if (--ref_counter == 0) {
            delete this;
        }
    }

    friend void swap(dynamic_buffer& a, dynamic_buffer& b) {
        using std::swap;
        swap(a.data, b.data);
        swap(a.ref_counter, b.ref_counter);
    }
private:
    size_t ref_counter;
};

#endif //BIGINT_DYNAMIC_BUFFER_H
