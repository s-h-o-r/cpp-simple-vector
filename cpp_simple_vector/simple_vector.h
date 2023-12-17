#pragma once

#include "array_ptr.h"
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <string>

class ReserveProxyObj {
public:
    ReserveProxyObj() = delete;

    ReserveProxyObj(size_t capacity_to_reserve)
        : capacity_to_reserve_(capacity_to_reserve) {
    }

    size_t GetNewCapacity() {
        return capacity_to_reserve_;
    }

private:
    size_t capacity_to_reserve_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size)
        : items_(ArrayPtr<Type>(size)),
          size_(size),
          capacity_(size) {
              Fill(begin(), end());
    }

    SimpleVector(size_t size, const Type& value)
        : items_(ArrayPtr<Type>(size)),
        size_(size),
        capacity_(size) {
            std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
        : items_(ArrayPtr<Type>(init.size())),
        size_(init.size()),
        capacity_(init.size()) {
            std::move(init.begin(), init.end(), begin());
    }

    SimpleVector(ReserveProxyObj new_capacity) {
            Reserve(new_capacity.GetNewCapacity());
    }

    SimpleVector(const SimpleVector& other)
        : items_(ArrayPtr<Type>(other.GetSize())),
        size_(other.GetSize()),
        capacity_(other.GetSize()) {
            std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other)
        : items_(other.capacity_) {
            swap(other);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector copy(rhs);
            swap(copy);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            swap(rhs);
        }
        return *this;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            IncreaseCapacity(new_capacity);
        }
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    Type& At(size_t index) {
        if (size_ <= index) {
            using namespace std::literals;
            throw std::out_of_range("Out of range."s);
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (size_ <= index) {
            using namespace std::literals;
            throw std::out_of_range("Out of range."s);
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        } else if (new_size > size_) {
            if (capacity_ < new_size) {
                IncreaseCapacity(std::max(new_size, capacity_ * 2));
            }
            Fill(begin() + size_, begin() + new_size);
            size_ = new_size;
        }
    }

    void PushBack(const Type& item) {
        if (size_ == capacity_) {
            IncreaseCapacity(std::max(size_t(1), capacity_ * 2));
        }
        items_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (size_ == capacity_) {
            IncreaseCapacity(std::max(size_t(1), capacity_ * 2));
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t distance = std::distance(cbegin(), pos);
        if (size_ == capacity_) {
            size_t new_capacity = std::max(static_cast<size_t>(1), capacity_ * 2);
            ArrayPtr<Type> new_array(new_capacity);
            std::move(begin(), items_.Get() + distance, new_array.Get());
            new_array[distance] = value;
            std::move(items_.Get() + distance, end(), new_array.Get() + (distance + 1));
            items_.swap(new_array);
            capacity_ = new_capacity;
        } else {
            std::move_backward(items_.Get() + distance, end(), end() + 1);
            *(begin() + distance) = value;
        }
        ++size_;
        return begin() + distance;
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t distance = std::distance(cbegin(), pos);
        if (size_ == capacity_) {
            size_t new_capacity = std::max(static_cast<size_t>(1), capacity_ * 2);
            ArrayPtr<Type> new_array(new_capacity);
            std::move(begin(), items_.Get() + distance, new_array.Get());
            new_array[distance] = std::move(value);
            std::move(items_.Get() + distance, end(), new_array.Get() + (distance + 1));
            items_.swap(new_array);
            capacity_ = new_capacity;
        } else {
            std::move_backward(items_.Get() + distance, end(), end() + 1);
            *(begin() + distance) = std::move(value);
        }
        ++size_;
        return begin() + distance;
    }

    void PopBack() noexcept {
        if (size_ != 0) {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos) {
        Iterator erasing_place = const_cast<Iterator>(pos);
        std::move(erasing_place + 1, end(), erasing_place);
        --size_;
        return erasing_place;
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }


    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }
private:
    ArrayPtr<Type> items_;

    size_t size_ = 0;
    size_t capacity_ = 0;

    void IncreaseCapacity(size_t new_capacity) {
        ArrayPtr<Type> new_array(new_capacity);
        std::move(begin(), end(), new_array.Get());
        items_.swap(new_array);
        capacity_ = new_capacity;
    }

    void Fill(Iterator first, Iterator last) {
        if (first < last) {
            for (; first != last; ++first)
                *first = std::move(Type{});
        }
    }
};

ReserveProxyObj Reserve(size_t new_capacity) {
    return ReserveProxyObj(new_capacity);
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {

    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs >= lhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
