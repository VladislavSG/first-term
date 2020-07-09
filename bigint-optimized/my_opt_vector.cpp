#include "my_opt_vector.h"

my_opt_vector::my_opt_vector() :
        size_(0),
        isSmall_(true) {}

my_opt_vector::my_opt_vector(my_opt_vector const &rhs) :
        size_(rhs.size_),
        isSmall_(rhs.isSmall_) {
    if (rhs.isSmall_) {
        std::copy_n(rhs.staticData_, rhs.size_, staticData_);
    } else {
        dynamicData_ = rhs.dynamicData_->makeCopy();
    }
}

my_opt_vector::~my_opt_vector() {
    if (!isSmall_) {
        dynamicData_->reduceCounter();
    }
}

my_opt_vector &my_opt_vector::operator=(my_opt_vector const &other) {
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

size_t my_opt_vector::size() const {
    return size_;
}

void my_opt_vector::pop_back() {
    unshare();
    resize(size_ - 1);
}

void my_opt_vector::push_back(uint32_t elem) {
    unshare();
    resize(size_ + 1);
    back() = elem;
}

uint32_t &my_opt_vector::back() {
    return operator[](size_ - 1);
}

uint32_t const &my_opt_vector::back() const {
    return operator[](size_ - 1);
}

uint32_t &my_opt_vector::operator[](size_t n) {
    if (isSmall_) {
        return staticData_[n];
    } else {
        unshare();
        return dynamicData_->data[n];
    }
}

uint32_t const &my_opt_vector::operator[](size_t n) const {
    if (isSmall_) {
        return staticData_[n];
    } else {
        return dynamicData_->data[n];
    }
}

void my_opt_vector::resize(size_t newSize) {
    if (isSmall_) {
        if (isSmall(newSize)) {
            if (newSize > size_) {
                std::fill(staticData_ + size_, staticData_ + newSize, 0u);
            }
        } else {
            dynamic_buffer *newData = new dynamic_buffer(newSize);
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

void my_opt_vector::unshare() {
    if (!isSmall_ && !dynamicData_->unique()) {
        dynamic_buffer *newData = new dynamic_buffer(dynamicData_->data);
        dynamicData_->reduceCounter();
        dynamicData_ = newData;
    }
}

bool my_opt_vector::isSmall(size_t x) {
    return x <= MAX_STATIC_SIZE;
}
