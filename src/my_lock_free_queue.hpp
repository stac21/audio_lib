/**************************************************************
 * @file queue.hpp
 * @brief A queue implementation written in standard c++11
 * suitable for both low-end microcontrollers all the way
 * to HPC machines. Lock-free for single consumer single
 * producer scenarios.
 **************************************************************/

 /**************************************************************
  * Copyright (c) 2023 Djordje Nedic
  *
  * Permission is hereby granted, free of charge, to any person
  * obtaining a copy of this software and associated
  * documentation files (the "Software"), to deal in the Software
  * without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to
  * whom the Software is furnished to do so, subject to the
  * following conditions:
  *
  * The above copyright notice and this permission notice shall
  * be included in all copies or substantial portions of the
  * Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
  * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
  * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
  * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *
  * This file is part of lockfree
  *
  * Author:          Djordje Nedic <nedic.djordje2@gmail.com>
  * Version:         v2.0.8
  **************************************************************/

#pragma once

#include <atomic>
#include <cstddef>
#include <type_traits>

// TODO rewrite the doxygen comments to be something that I wrote

/*
 * the lock free queue implementation is a circular buffer where the value is
 * never actually "popped" but rather overwritten. the read and write index
 * both start at 0, and the write index is incremented when a value is pushed
 * to the queue. To check whether the queue is full when pushing, you check
 * the index which you will be writing to. If it is equal to the read index,
 * then the queue is full. Else, write to that index and increment the write
 * index member variable
 */

namespace lockfree {
namespace spsc {
template <typename T, size_t size> class Queue {
    static_assert(std::is_trivial<T>::value, "The type T must be trivial");
    static_assert(size > 2, "Buffer size must be bigger than 2");

private:
    // TODO remove this comment. There is no need for a std::array, it only adds to the struct size
    T elements[size];

#if LOCKFREE_CACHE_COHERENT
    alignas(LOCKFREE_CACHELINE_LENGTH) std::atomic<size_t> read_index;
    alignas(
        LOCKFREE_CACHELINE_LENGTH) std::atomic<size_t> write_index;
#else
    std::atomic<size_t> read_index;
    std::atomic<size_t> write_index;
#endif

public:
    Queue() :
        read_index(0),
        write_index(0)
    {}

    template<typename T2>
    Queue(const Queue<T2, size>& rhs) requires std::convertible_to<T2, T> = default;
    template<typename T2>
    const Queue<T, size> operator=(const Queue<T2, size>& rhs) requires std::convertible_to<T2, T> = default;

    ~Queue() = default;

    /**
     * @brief Adds an element into the queue.
     * Should only be called from the producer thread.
     * @param[in] element
     * @return Operation success
     */
    // TODO this should probably be a universal reference
    bool push(const T& element);

    /**
     * @brief Removes an element from the queue.
     * Should only be called from the consumer thread.
     * @param[out] element
     * @return Whether a value was popped from the queue
     */
    bool pop();

    const T& peek() const noexcept {
        return this->elements[this->read_index];
    }

    /**
     * @brief Return the max size of the queue
     * @return Max size of the queue
     */
    constexpr size_t capacity() noexcept {
        return this->size;
    }

    /**
     * @brief Return whether the queue is empty
     * @return Whether the queue is empty
     */
    bool is_empty() noexcept {
        // TODO fill this in with the actual logic
        return false;
    }
};

} /* namespace spsc */
} /* namespace lockfree */
