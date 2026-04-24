#pragma once
#include <cstdint>

namespace tdsys {
    using Price = int64_t;
    using Qty = int64_t;
    using OrderId = uint64_t;

    enum class Side : uint8_t { BUY, SELL };
    enum class OrderType : uint8_t { LIMIT, MARKET, IOC, FOK };

    struct Order {
        OrderId id{};
        Side side{};
        OrderType type{};
        Price price{};
        Qty qty{};
        Qty filled{};
        uint64_t ts{};
    };
}