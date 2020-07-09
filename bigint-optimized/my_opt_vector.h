#ifndef BIGINT_MY_OPT_VECTOR_H
#define BIGINT_MY_OPT_VECTOR_H

#include <stdexcept>
#include <memory>
#include <algorithm>
#include "dynamic_buffer.h"

struct my_opt_vector {
    my_opt_vector();

    my_opt_vector(my_opt_vector const& rhs);

    ~my_opt_vector();

    my_opt_vector& operator=(my_opt_vector const& other);

    size_t size() const;

    void pop_back();

    void push_back(uint32_t elem);

    uint32_t& back();

    uint32_t const& back() const;

    uint32_t& operator[](size_t n);

    uint32_t const& operator[](size_t n) const;

    void resize(size_t newSize);

private:
    size_t size_;
    bool isSmall_;
    static constexpr size_t MAX_STATIC_SIZE = 8;
    union {
        dynamic_buffer* dynamicData_;
        uint32_t staticData_[MAX_STATIC_SIZE];
    };

    static bool isSmall(size_t x);

    void unshare();
};

#endif //BIGINT_MY_OPT_VECTOR_H
