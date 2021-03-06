#ifndef VECTOR_H
#define VECTOR_H

#include <cstddef>
#include <memory.h>
#include <cassert>
#include <utility>
#include <cstdint>

template <typename T>
struct vector
{
    typedef T* iterator;
    typedef T const* const_iterator;

    const uint8_t UPPER_BOUND = 2; // if (size_ == capacity) then capacity_ = UPPER_BOUND * capacity_
    const uint8_t LOWER_BOUND = 4; // if (size_ < capacity / LOWER_BOUND) then capacity_ = UPPER_BOUND * size_

    // O(1) nothrow
    vector() :
        data_(nullptr),
        size_(0),
        capacity_(0)
    {}

    // O(N) strong
    vector(vector const& other) : vector() {
        data_ = alloc_data(other.size_);
        save_copy(other.data_, data_, 0, other.size_);
        capacity_ = other.size_;
        size_ = other.size_;
    }

    // O(N) strong
    vector& operator=(vector const& other) {
        vector temp(other);
        swap(temp);
        return *this;
    }

    // O(N) nothrow
    ~vector() {
        destruct_all();
        operator delete(data_);
    }

    // O(1) nothrow
    T& operator[](size_t i) {
        return data_[i];
    }

    // O(1) nothrow
    T const& operator[](size_t i) const {
        return data_[i];
    }

    // O(1) nothrow
    T* data() {
        return data_;
    }

    // O(1) nothrow
    T const* data() const {
        return data_;
    }

    // O(1) nothrow
    size_t size() const {
        return size_;
    }

    // O(1) nothrow
    T& front() {
        return data_[0];
    }

    // O(1) nothrow
    T const& front() const {
        return data_[0];
    }

    // O(1) nothrow
    T& back() {
        return data_[size_ - 1];
    }

    // O(1) nothrow
    T const& back() const {
        return data_[size_ - 1];
    }

    // O(1) strong
    void push_back(T const& element) {
        insert(data_ + size_, element);
    }

    // O(1) nothrow
    void pop_back() {
        assert(size_ != 0);

        erase(data_ + size_ - 1);
    }

    // O(1) nothrow
    bool empty() const {
        return size_ == 0;
    }

    // O(1) nothrow
    size_t capacity() const {
        return capacity_;
    }

    // O(N) strong
    void reserve(size_t n) {
        if (n > capacity_)
            new_buffer(n);
    }

    // O(N) strong
    void shrink_to_fit() {
        if (size_ != capacity_)
            new_buffer(size_);
    }

    // O(N) nothrow
    void clear() {
        destruct_all();
        size_ = 0;
    }

    // O(1) nothrow
    void swap(vector& other) {
        using std::swap;

        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

    // O(1) nothrow
    iterator begin() {
        return data_;
    }

    // O(1) nothrow
    iterator end() {
        return data_ + size_;
    }

    // O(1) nothrow
    const_iterator begin() const {
        return data_;
    }

    // O(1) nothrow
    const_iterator end() const {
        return data_ + size_;
    }

    // O(N) weak
    iterator insert(const_iterator it, T const& element) {
        size_t position = it - data_;
        if (size_ != capacity_) {
            T bubble(element);
            for (size_t i = position; i != size_; ++i) {
                std::swap(bubble, data_[i]);
            }
            new(data_ + size_) T(bubble);
        } else {
            size_t new_capacity = (capacity_ ? capacity_ * UPPER_BOUND : 1);
            T *new_data = alloc_data(new_capacity);

            save_copy(data_, new_data, 0, position);
            try {
                new(new_data + position) T(element);
            } catch (...) {
                destruct_all(new_data, position + 1);
                operator delete(new_data);
                throw;
            }
            save_copy(data_ + position, new_data, position + 1, size_ - position);

            destruct_all();
            operator delete(data_);
            data_ = new_data;
            capacity_ = new_capacity;
        }
        ++size_;
        return data_ + position;
    }

    // O(N) weak
    iterator erase(const_iterator pos) {
        return erase(pos, pos + 1);
    }

    // O(N) weak
    iterator erase(const_iterator first, const_iterator last) {
        size_t count_delete = last - first;
        if (count_delete) {
            size_t new_size = size_ - count_delete;
            size_t left_count = first - data_;
            size_t right_count = data_ + size_ - last;
            assert(count_delete <= size_);

            if (right_count == 0 && new_size >= capacity_ / LOWER_BOUND) {
                destruct_all(data_ + left_count, count_delete);
            } else {
                size_t new_capacity = new_size * UPPER_BOUND;           // not a zero
                T *new_data = alloc_data(new_capacity);

                save_copy(data_, new_data, 0, left_count);
                save_copy(data_ + left_count + count_delete, new_data, left_count, right_count);

                destruct_all();
                operator delete(data_);
                data_ = new_data;
                capacity_ = new_capacity;
            }
            size_ = new_size;
        }
        return const_cast<iterator>(first);
    }

private:
    void new_buffer(size_t new_capacity) {
        T* new_data = alloc_data(new_capacity);
        size_t new_size = (size_ > new_capacity ? new_capacity : size_);
        save_copy(data_, new_data, 0, new_size);
        destruct_all();
        operator delete(data_);
        data_ = new_data;
        capacity_ = new_capacity;
        size_ = new_size;
    }

    void destruct_all() {
        destruct_all(data_, size_);
    }

    static void destruct_all(T* array, size_t count) {
        assert(count <= PTRDIFF_MAX);

        for (ptrdiff_t i = count - 1; i != -1; --i)
            array[i].~T();
    }

    static void save_copy(T const* from, T * &to, size_t to_start, size_t count) {
        size_t count_copied = 0;
        try {
            for (size_t i = 0; i < count; i++) {
                new(to + to_start + i) T(from[i]);
                ++count_copied;
            }
        } catch(...) {
            destruct_all(to, to_start + count_copied);
            operator delete(to);
            to = nullptr;
            throw;
        }
    }

    static T* alloc_data (size_t count) {
        if (count) {
            return static_cast<T*>(operator new(count * sizeof(T)));
        } else {
            return nullptr;
        }
    }

private:
    T* data_;
    size_t size_;
    size_t capacity_;
};

#endif // VECTOR_H
