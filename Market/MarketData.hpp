#pragma once
#include <cstdint>
#include "Common.hpp"

namespace tdsys::market {

    struct MarketTick {
        u64 seq;
        u64 ts;
        i64 bid_price;
        i64 ask_price;
        i64 bid_qty;
        i64 ask_qty;
    };

}