#pragma once
#include "Market/MarketData.hpp"
#include "Gateway/OrderGateway.hpp"
#include "Risk/RiskManager.hpp"

namespace tdsys::strategy {

    class BaseStrategy {
    public:
        virtual void on_tick(const market::MarketTick& tick) = 0;
    };

    class StrategyEngine {
    public:
        gateway::OrderGateway gw;
        risk::RiskManager risk;

        void run(BaseStrategy& s) {
            market::MarketTick tick;
            while (market::MarketGateway{}.tick_queue.pop(tick)) {
                s.on_tick(tick);
            }
        }
    };

}
