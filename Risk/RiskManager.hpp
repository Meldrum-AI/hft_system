#pragma once
#include <atomic>
#include "OrderBook/Order.hpp"

namespace tdsys::risk {

    class RiskManager {
    private:
        std::atomic<i64> position{ 0 };
        const i64 MAX_POS = 200;
    public:
        bool check(Side side, Qty qty) {
            i64 delta = (side == Side::BUY) ? qty : -qty;
            if (position + delta > MAX_POS || position + delta < -MAX_POS)
                return false;
            return true;
        }

        void update(Side side, Qty qty) {
            if (side == Side::BUY)
                position += qty;
            else
                position -= qty;
        }
    };

}