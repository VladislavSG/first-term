#ifndef BIGINT_MY_OPT_VECTOR_H
#define BIGINT_MY_OPT_VECTOR_H

#include <vector>
#include <memory>
#include <algorithm>

template <typename T>
struct my_opt_vector {
    my_opt_vector() :
                size_(0),
                data_(operator new(MAX_STATIC_SIZE * sizeof(uint32_t))),
                isSmall_(true) {};

    my_opt_vector(my_opt_vector const& rhs) :
                size_(rhs.size_),
                data_(operator new(MAX_STATIC_SIZE * sizeof(uint32_t))),
                isSmall_(rhs.isSmall_) {
        if (isSmall_) {
            std::copy_n(rhs.getStatic(), rhs.size_, getStatic());
        } else {
            *changeToDynamic() = *rhs.getDynamic();
        }
    }

    ~my_opt_vector() {
        if (!isSmall_) {
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
        swap(size_, other.size_);
        swap(data_, other.data_);
        swap(isSmall_, other.isSmall_);
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
            return getStatic()[n];
        } else {
            unshare();
            return (**getDynamic())[n];
        }
    }

    T const& operator[](size_t n) const {
        if (isSmall_) {
            return getStatic()[n];
        } else {
            return (**getDynamic())[n];
        }
    }

    void resize(size_t newSize) {
        if (isSmall_) {
            if (isSmall(newSize)) {
                if (newSize > size_) {
                    std::fill(getStatic() + size_, getStatic() + newSize, 0u);
                }
            } else {
                auto* buffer = new std::vector<T>(newSize);
                std::copy_n(getStatic(), size_, buffer->begin());
                changeToDynamic(buffer);
            }
        } else {
            unshare();
            (*getDynamic())->resize(newSize);
        }
        size_ = newSize;
    }

private:
    typedef std::shared_ptr<std::vector<T>> dynamic_t;
    size_t size_;
    void* data_;
    bool isSmall_;
    static const size_t MAX_STATIC_SIZE = sizeof(dynamic_t) / sizeof(T) + 8;

    static bool isSmall(size_t x) {
        return x <= MAX_STATIC_SIZE;
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
        isSmall_ = false;
        return dynamicPtr;
    }

    void unshare() {
        if (!isSmall_ && !getDynamic()->unique())
            getDynamic()->reset(new std::vector<T>(**getDynamic()));
    }
};

template struct my_opt_vector<uint32_t>;

#endif //BIGINT_MY_OPT_VECTOR_H
