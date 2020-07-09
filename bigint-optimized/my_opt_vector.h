#ifndef BIGINT_MY_OPT_VECTOR_H
#define BIGINT_MY_OPT_VECTOR_H

#include <vector>
#include <memory>
#include <algorithm>

template <typename T>
struct my_opt_vector {
    my_opt_vector() :
                size_(0),
                data_(operator new(sizeof(dynamic_t))) {};

    my_opt_vector(my_opt_vector const& rhs) :
                size_(rhs.size_),
                data_(operator new(sizeof(dynamic_t))) {
        if (isSmall()) {
            std::copy_n(rhs.getStatic(), rhs.size_, getStatic());
        } else {
            *changeToDynamic() = *rhs.getDynamic();
        }
    }

    ~my_opt_vector() {
        if (!isSmall()) {
            getDynamic()->~dynamic_t();
        }
        operator delete(data_);
    }

    my_opt_vector& operator=(my_opt_vector const& other) {
        my_opt_vector temp(other);
        swap(temp);
        return *this;
    }

    void swap(my_opt_vector& other) {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
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
            return getStatic()[n];
        } else {
            unshare();
            return (**getDynamic())[n];
        }
    }

    T const& operator[](size_t n) const {
        if (isSmall()) {
            return getStatic()[n];
        } else {
            return (**getDynamic())[n];
        }
    }

    void resize(size_t newSize) {
        if (isSmall(newSize)) {
            if (isSmall()) {
                if (newSize > size_) {
                    std::fill(getStatic() + size_, getStatic() + newSize, 0u);
                }
            } else {
                T buffer[MAX_STATIC_SIZE];
                std::copy_n((*getDynamic())->begin(), newSize, buffer);
                changeToStatic();
                std::copy_n(buffer, newSize, getStatic());
            }
        } else {
            if (isSmall()) {
                auto* buffer = new std::vector<T>(newSize);
                std::copy_n(getStatic(), size_, buffer->begin());
                changeToDynamic(buffer);
            } else {
                unshare();
                (*getDynamic())->resize(newSize);
            }
        }
        size_ = newSize;
    }

private:
    typedef std::shared_ptr<std::vector<T>> dynamic_t;
    size_t size_;
    static const size_t MAX_STATIC_SIZE = sizeof(dynamic_t) / sizeof(T);
    void* data_;

    static bool isSmall(size_t x) {
        return x <= MAX_STATIC_SIZE;
    }

    bool isSmall() const {
        return isSmall(size_);
    }

    dynamic_t* getDynamic() const {
        return static_cast<dynamic_t*>(data_);
    }

    T* getStatic() const {
        return static_cast<T*>(data_);
    }

    dynamic_t* changeToDynamic(std::vector<T>* ptr = nullptr) {
        dynamic_t* dynamicPtr = getDynamic();
        new(dynamicPtr) dynamic_t(ptr);
        return dynamicPtr;
    }

    T* changeToStatic() {
        dynamic_t* dynamicPtr = getDynamic();
        dynamicPtr->~dynamic_t();
        return getStatic();
    }

    void unshare() {
        if (!isSmall() && !getDynamic()->unique())
            getDynamic()->reset(new std::vector<T>(**getDynamic()));
    }
};

template struct my_opt_vector<uint32_t>;

#endif //BIGINT_MY_OPT_VECTOR_H
