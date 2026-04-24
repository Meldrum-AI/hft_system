#pragma once
#include <atomic>
#include <array>
#include <semaphore.h>

namespace tdsys {
    template<typename T, size_t SZ = 127>
    class MsgQueue {
    private:
        static constexpr size_t LEN = 1 << std::bit_width(SZ);
        static constexpr size_t MASK = LEN - 1;
        static constexpr size_t IB = 16;
        static constexpr size_t IM = (1ULL << IB) - 1;

        struct W {
            std::atomic<bool> ok{ false };
            T data{};
        };

        alignas(64) std::atomic<size_t> h{ 0 }, vt{ 0 };
        sem_t sem{};
        std::array<W, LEN> buf{};

    public:
        MsgQueue() { sem_init(&sem, 1, 0); }
        ~MsgQueue() { sem_destroy(&sem); }

        template<typename U>
        bool push(U&& u) {
            size_t v;
            while (true) {
                auto lvt = vt.load(std::memory_order_acquire);
                auto lt = lvt & IM;
                auto n = (lt + 1) & MASK;
                auto hh = h.load(std::memory_order_acquire) & IM;
                if (n == hh) return false;
                v = (lvt >> IB) + 1;
                size_t nvt = (v << IB) | n;
                if (vt.compare_exchange_strong(lvt, nvt, std::memory_order_acq_rel)) {
                    auto& w = buf[lt];
                    w.data = std::forward<U>(u);
                    w.ok.store(true, std::memory_order_release);
                    sem_post(&sem);
                    return true;
                }
            }
        }

        bool pop(T& out) {
            sem_wait(&sem);
            auto lh = h.load(std::memory_order_relaxed);
            auto tail = vt.load(std::memory_order_acquire) & IM;
            if (lh == tail) return false;
            auto& w = buf[lh];
            if (!w.ok.load(std::memory_order_acquire)) return false;
            out = std::move(w.data);
            w.ok.store(false, std::memory_order_release);
            h.store((lh + 1) & MASK, std::memory_order_release);
            return true;
        }
    };
}
