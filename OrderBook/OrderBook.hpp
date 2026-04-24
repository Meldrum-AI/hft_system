#pragma once
#include "Order.hpp"
#include "../Memory/SlabPool.hpp"
#include "../LockFree/SPSCQueue.hpp"
#include <map>

namespace tdsys {
    class OrderBook {
    private:
        using BookLevel = SPSCQueue<Order*, 4096>;
        using BookMap = std::map<Price, BookLevel, std::greater<>>;

        BookMap bids;
        BookMap asks;
        SlabPool<Order> pool;
        OrderId next_id = 1;

    public:
        Order* create(Side s, OrderType t, Price p, Qty q) {
            Order* o = pool.alloc();
            o->id = next_id++;
            o->side = s;
            o->type = t;
            o->price = p;
            o->qty = q;
            o->filled = 0;
            o->ts = __rdtsc();
            return o;
        }

        void add(Order* o) {
            if (o->side == Side::BUY)
                bids[o->price].push(o);
            else
                asks[o->price].push(o);
        }

        void match() {
            while (!bids.empty() && !asks.empty()) {
                auto& best_bid = bids.begin();
                auto& best_ask = asks.begin();

                Price bp = best_bid->first;
                Price ap = best_ask->first;
                if (bp < ap) break;

                Order* b, * a;
                if (!best_bid->second.pop(b) || !best_ask->second.pop(a))
                    break;

                Qty trade_qty = std::min(b->qty - b->filled, a->qty - a->filled);
                b->filled += trade_qty;
                a->filled += trade_qty;
            }
        }
    };
}
