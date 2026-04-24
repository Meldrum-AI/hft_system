#pragma once
#include <atomic>
#include <array>

namespace tdsys {
    template<typename T, size_t SZ = 127>
    class SPMCQueue {
    private:
        static constexpr size_t LEN = 1 << std::bit_width(SZ);
        static constexpr size_t MASK = LEN - 1;
        static constexpr size_t IB = 16;
        static constexpr size_t IM = (1ULL << IB) - 1;
        alignas(64) std::atomic<size_t> vh{ 0 }, t{ 0 };
        std::array<T, LEN> buf{};

    public:
        template<typename U>
        bool push(U&& u) {
            auto lt = t.load(std::memory_order_relaxed);
            auto n = (lt + 1) & MASK;
            auto hh = vh.load(std::memory_order_acquire) & IM;
            if (n == hh) return false;
            buf[lt] = std::forward<U>(u);
            t.store(n, std::memory_order_release);
            return true;
        }

        bool pop(T& out) {
            while (true) {
                auto lvh = vh.load(std::memory_order_acquire);
                auto lh = lvh & IM;
                auto lt = t.load(std::memory_order_relaxed);
                if (lh == lt) return false;
                size_t nv = (lvh >> IB) + 1;
                size_t nvh = (nv << IB) | ((lh + 1) & MASK);
                if (vh.compare_exchange_strong(lvh, nvh, std::memory_order_acq_rel)) {
                    out = std::move(buf[lh]);
                    return true;
                }
            }
        }
    };
}