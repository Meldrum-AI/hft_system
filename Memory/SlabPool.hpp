#pragma once
#include <cstddef>
#include <cstdlib>

namespace tdsys {
    template<typename T, size_t CAP = 1 << 16>
    class SlabPool {
    private:
        char mem[sizeof(T) * CAP]{};
        std::atomic<size_t> idx{ 0 };
    public:
        T* alloc() {
            auto i = idx.fetch_add(1, std::memory_order_relaxed);
            return i < CAP ? (T*)(mem + i * sizeof(T)) : nullptr;
        }
    };
}