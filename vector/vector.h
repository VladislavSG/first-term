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
    {};
    // O(N) strong
    vector(vector const& other) : vector() {
        data_ = (other.size_ ? alloc_data(other.size_) : nullptr);
        size_t count_copied = 0;
        try {
            for (size_t i = 0; i < other.size_; ++i) {
                new(data_ + i) T(other[i]);
                ++count_copied;
            }
            capacity_ = other.size_;
            size_ = other.size_;
        } catch (...) {
            for (T* i = data_ + count_copied - 1; i != data_ -1; i--) {
                i->~T();
            }
            operator delete(data_);
            data_ = nullptr;
            throw;
        }
    };
    // O(N) strong
    vector& operator=(vector const& other) {
        vector temp(other);
        swap(temp);
        return *this;
    };
    // O(N) nothrow
    ~vector() {
        destruct_all();
        operator delete(data_);
    };

    // O(1) nothrow
    T& operator[](size_t i) {
        return data_[i];
    };
    // O(1) nothrow
    T const& operator[](size_t i) const {
        return data_[i];
    };

    // O(1) nothrow
    T* data() {
        return data_;
    };
    // O(1) nothrow
    T const* data() const {
        return data_;
    };
    // O(1) nothrow
    size_t size() const {
        return size_;
    };

    // O(1) nothrow
    T& front() {
        return data_[0];
    };
    // O(1) nothrow
    T const& front() const {
        return data_[0];
    };

    // O(1) nothrow
    T& back() {
        return data_[size_ - 1];
    };
    // O(1) nothrow
    T const& back() const {
        return data_[size_ - 1];
    };
    // O(1) strong
    void push_back(T const& element) {
        insert(end(), element);
    };
    // O(1) nothrow
    void pop_back() {
        assert(size_ != 0);

        erase(end() - 1);
    };

    // O(1) nothrow
    bool empty() const {
        return size_ == 0;
    };

    // O(1) nothrow
    size_t capacity() const {
        return capacity_;
    };
    // O(N) strong
    void reserve(size_t n) {
        if (n > capacity_)
            new_buffer(n);
    };
    // O(N) strong
    void shrink_to_fit() {
        if (size_ == 0)
            data_ = nullptr;
        else if (size_ != capacity_)
            new_buffer(size_);
    };
    // O(N) nothrow
    void clear() {
        destruct_all();
        size_ = 0;
    };

    void destruct_all() {
        if (data_ != nullptr) {
            for (T *i = data_ + size_ - 1; i != data_ - 1; i--)
                i->~T();
        }
    }

    // O(1) nothrow
    void swap(vector& other) {
        using std::swap;

        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    };

    // O(1) nothrow
    iterator begin() {
        return data_;
    };
    // O(1) nothrow
    iterator end() {
        return data_ + size_;
    };

    // O(1) nothrow
    const_iterator begin() const {
        return data_;
    };
    // O(1) nothrow
    const_iterator end() const {
        return data_ + size_;
    };

    // O(N) weak
    iterator insert(iterator it, T const& element) {
        return insert(static_cast<const_iterator>(it), element);
    };
    // O(N) weak
    iterator insert(const_iterator it, T const& element) {
        size_t position = it - begin();
        if (size_ != capacity_) {
            auto new_iterator = const_cast<iterator>(it);
            for (iterator i = end(); i != new_iterator; i--) {
                new(i) T(*(i-1));
                (i-1)->~T();
            }
            new(new_iterator) T(element);
        } else {
            size_t new_capacity = (capacity_ ? capacity_ * UPPER_BOUND : 1);
            T *new_data = alloc_data(new_capacity);
            size_t count_copied = 0;
            try {
                size_t i;
                for (i = 0; i != position; ++i) {
                    new(new_data + i) T(operator[](i));
                    ++count_copied;
                }
                new(new_data + i) T(element);
                for (++i; i < size_ + 1; ++i) {
                    new(new_data + i) T(operator[](i - 1));
                    ++count_copied;
                }
            } catch (...) {
                for (T *i = new_data + count_copied; i != new_data; --i)
                    (i - 1)->~T();
                operator delete(new_data);
                throw;
            }
            destruct_all();
            operator delete(data_);
            data_ = new_data;
            capacity_ = new_capacity;
        }
        ++size_;
        return data_ + position;
    };

    // O(N) weak
    iterator erase(iterator pos) {
        return erase(pos, static_cast<T*>(pos) + 1);
    };
    // O(N) weak
    iterator erase(const_iterator pos) {
        return erase(pos, static_cast<T const*>(pos) + 1);
    };

    // O(N) weak
    iterator erase(iterator first, iterator last) {
        return erase(static_cast<const_iterator>(first), static_cast<const_iterator>(last));
    };
    // O(N) weak
    iterator erase(const_iterator first, const_iterator last) {
        size_t count_delete = last - first;
        size_t new_size = size_ - count_delete;
        assert(count_delete <= size_);
        if (new_size >= capacity_ / LOWER_BOUND) {
            for (auto *i = const_cast<iterator>(first); i != const_cast<iterator>(last); ++i) {
                i->~T();
            }
            for (auto* i = const_cast<iterator>(last), *j = const_cast<iterator>(first); i != end(); ++i, ++j) {
                new(j) T(*i);
                i->~T();
            }
        } else {
            size_t new_capacity = new_size * UPPER_BOUND;
            T* new_data = alloc_data(new_capacity);
            size_t count_copied = 0;
            try {
                size_t i;
                for (i = 0; i < first - begin(); i++) {
                    new(new_data + i) T(data_[i]);
                    ++count_copied;
                }
                for (i += count_delete; i < size_; i++) {
                    new(new_data + i - count_delete) T(data_[i]);
                    ++count_copied;
                }
            } catch (...) {
                for (T* i = new_data + count_copied; i != new_data; i--)
                    (i-1)->~T();
                operator delete(new_data);
                throw;
            }
            destruct_all();
            operator delete(data_);
            data_ = new_data;
            capacity_ = new_capacity;
        }
        size_ = new_size;
        return const_cast<iterator>(first);
    };

private:
    void new_buffer(size_t new_capacity) {
        T* new_data = alloc_data(new_capacity);
        size_t new_size;
        if (size_ > new_capacity) {
            new_size = new_capacity;
        } else {
            new_size = size_;
        }
        size_t count_copied = 0;
        try {
            for (size_t i = 0; i < new_size; i++) {
                new(new_data + i) T(data_[i]);
                ++count_copied;
            }
        } catch(...) {
            for (T* i = new_data + count_copied; i != new_data; i--)
                (i-1)->~T();
            operator delete(new_data);
            throw;
        }
        destruct_all();
        operator delete(data_);
        data_ = new_data;
        capacity_ = new_capacity;
        size_ = new_size;
    };

    T* alloc_data (size_t count) {
        if (count) {
            return reinterpret_cast<T *>(operator new(count * sizeof(T)));
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
