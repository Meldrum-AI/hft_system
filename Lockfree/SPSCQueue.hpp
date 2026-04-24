#pragma once
#include <atomic>
#include <array>

namespace tdsys {
    template<typename T, size_t SZ = 127>
    class SPSCQueue {
    private:
        static constexpr size_t LEN = 1 << std::bit_width(SZ);
        static constexpr size_t MASK = LEN - 1;
        alignas(64) std::atomic<size_t> h{ 0 }, t{ 0 };
        alignas(64) size_t sh = 0, st = 0;
        std::array<T, LEN> buf{};

    public:
        bool pop(T& out) {
            auto lh = h.load(std::memory_order_relaxed);
            if (lh != st && (st = t.load(std::memory_order_acquire)) != lh) {
                out = std::move(buf[lh]);
                h.store((lh + 1) & MASK, std::memory_order_release);
                return true;
            }
            return false;
        }

        template<typename U>
        bool push(U&& u) {
            auto lt = t.load(std::memory_order_relaxed);
            auto n = (lt + 1) & MASK;
            if (n != sh && (sh = h.load(std::memory_order_acquire)) != n) {
                buf[lt] = std::forward<U>(u);
                t.store((lt + 1) & MASK, std::memory_order_release);
                return true;
            }
            return false;
        }
    };
}