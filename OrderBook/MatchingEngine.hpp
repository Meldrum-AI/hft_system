#pragma once
#include "OrderBook.hpp"

namespace tdsys {
    class MatchingEngine {
    private:
        OrderBook& ob;
    public:
        MatchingEngine(OrderBook& ob_) : ob(ob_) {}

        void run_once() {
            ob.match();
        }
    };
}