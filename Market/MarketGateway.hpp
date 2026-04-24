#pragma once
#include "MarketData.hpp"
#include "SPMCQueue.hpp"

namespace tdsys::market {

    class MarketGateway {
    public:
        SPMCQueue<MarketTick, 4096> tick_queue;

        void push_tick(const MarketTick& tick) {
            tick_queue.push(tick);
        }
    };

}
