#ifndef BIGINT_MY_OPT_VECTOR_H
#define BIGINT_MY_OPT_VECTOR_H

#include <vector>
#include <memory>
#include <algorithm>
#include "dynamic_buffer.h"

template <typename T>
struct my_opt_vector {
    my_opt_vector() :
                size_(0),
                isSmall_(true) {
        staticData_[0] = 0;
    };

    my_opt_vector(my_opt_vector const& rhs) :
                size_(rhs.size_),
                isSmall_(rhs.isSmall_) {
        if (rhs.isSmall_) {
            std::copy_n(rhs.staticData_, rhs.size_, staticData_);
        } else {
            dynamicData_ = rhs.dynamicData_->makeCopy();
        }
    }

    ~my_opt_vector() {
        if (!isSmall_) {
            dynamicData_->reduceCounter();
        }
    }

    my_opt_vector& operator=(my_opt_vector const& other) {
        if (this != &other) {
            if (!isSmall_) {
                dynamicData_->reduceCounter();
            }
            size_ = other.size_;
            isSmall_ = other.isSmall_;
            if (other.isSmall_) {
                std::copy_n(other.staticData_, size_, staticData_);
            } else {
                dynamicData_ = other.dynamicData_->makeCopy();
            }
        }
        return *this;
    }

    size_t size() const {
        return size_;
    }

    void pop_back() {
        unshare();
        resize(size_ - 1);
    }

    void push_back(T elem) {
        unshare();
        resize(size_ + 1);
        back() = elem;
    }

    T& back() {
        return operator[](size_ - 1);
    }

    T const& back() const {
        return operator[](size_ - 1);
    }

    T& operator[](size_t n) {
        if (isSmall_) {
            return staticData_[n];
        } else {
            unshare();
            return dynamicData_->data[n];
        }
    }

    T const& operator[](size_t n) const {
        if (isSmall_) {
            return staticData_[n];
        } else {
            return dynamicData_->data[n];
        }
    }

    void resize(size_t newSize) {
        if (isSmall_) {
            if (isSmall(newSize)) {
                if (newSize > size_) {
                    std::fill(staticData_ + size_, staticData_ + newSize, 0u);
                }
            } else {
                dynamic_buffer<T>* newData = new dynamic_buffer<T>(newSize);
                std::copy_n(staticData_, size_, newData->data.begin());
                dynamicData_ = newData;
                isSmall_ = false;
            }
        } else {
            unshare();
            dynamicData_->data.resize(newSize);
        }
        size_ = newSize;
    }

private:
    size_t size_;
    bool isSmall_;
    static const size_t MAX_STATIC_SIZE = 2;
    union {
        dynamic_buffer<T>* dynamicData_;
        T staticData_[MAX_STATIC_SIZE];
    };

    static bool isSmall(size_t x) {
        return x <= MAX_STATIC_SIZE;
    }

    void unshare() {
        if (!isSmall_ && !dynamicData_->unique()) {
            dynamic_buffer<T>* newData = new dynamic_buffer<T>(dynamicData_->data);
            dynamicData_->reduceCounter();
            dynamicData_ = newData;
        }
    }
};

template struct my_opt_vector<uint32_t>;

#endif //BIGINT_MY_OPT_VECTOR_H
