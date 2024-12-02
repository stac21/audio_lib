#pragma once

#include <array>
#include <atomic>
#include <lockfree/lockfree.hpp>

template<typename T, size_t Size>
using LockFreeQueue = lockfree::spsc::Queue<T, Size>;
