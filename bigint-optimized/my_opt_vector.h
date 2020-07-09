#ifndef BIGINT_MY_OPT_VECTOR_H
#define BIGINT_MY_OPT_VECTOR_H

#include <vector>
#include <memory>
#include <algorithm>

template <typename T>
struct my_opt_vector {
    my_opt_vector() : size_(0) {};

    my_opt_vector(my_opt_vector const& rhs) : size_(rhs.size_) {
        if (isSmall()) {
            std::copy_n(rhs.staticData_, rhs.size_, staticData_);
        } else {
            dynamicData_ = rhs.dynamicData_;
        }
    }

    ~my_opt_vector() {
        if (!isSmall()) {
            dynamicData_.reset();
        }
    }

    my_opt_vector& operator=(my_opt_vector const& other) {
        if (this != &other) {
            if (!isSmall(size_)) {
                dynamicData_.reset();
            }
            size_ = other.size_;
            if (other.isSmall()) {
                std::copy_n(other.staticData_, other.size_, staticData_);
            } else {
                dynamicData_ = other.dynamicData_;
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
        if (isSmall()) {
            return staticData_[n];
        } else {
            unshare();
            return (*dynamicData_)[n];
        }
    }

    T const& operator[](size_t n) const {
        if (isSmall()) {
            return staticData_[n];
        } else {
            return (*dynamicData_)[n];
        }
    }

    void resize(size_t newSize) {
        if (isSmall(newSize)) {
            if (isSmall()) {
                if (newSize > size_) {
                    std::fill(staticData_ + size_, staticData_ + newSize, 0);
                }
            } else {
                T buffer[MAX_STATIC_SIZE];
                std::copy_n(dynamicData_->begin(), newSize, buffer);
                dynamicData_.reset();
                std::copy_n(buffer, newSize, staticData_);
            }
        } else {
            if (isSmall()) {
                auto* buffer = new std::vector<T>(newSize);
                std::copy_n(staticData_, size_, buffer->begin());
                dynamicData_.reset(buffer);
            } else {
                dynamicData_->resize(newSize);
            }
        }
        size_ = newSize;
    }

private:
    typedef std::shared_ptr<std::vector<T>> dynamic_t;
    size_t size_;
    static const size_t MAX_STATIC_SIZE = sizeof(dynamic_t) / sizeof(T);
    union {
        dynamic_t dynamicData_;
        T staticData_[MAX_STATIC_SIZE];
    };

    static bool isSmall(size_t x) {
        return x <= MAX_STATIC_SIZE;
    }

    bool isSmall() const {
        return isSmall(size_);
    }

    void unshare() {
        if (!isSmall() && !dynamicData_.unique())
            dynamicData_.reset(new std::vector<T>(*dynamicData_));
    }
};

template struct my_opt_vector<uint32_t>;

#endif //BIGINT_MY_OPT_VECTOR_H
