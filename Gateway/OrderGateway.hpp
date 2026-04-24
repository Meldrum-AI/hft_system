#pragma once
#include "OrderBook/Order.hpp"
#include "LockFree/MPSCQueue.hpp"

namespace tdsys::gateway {

    struct OrderRequest {
        u64 id;
        Side side;
        Price price;
        Qty qty;
    };

    class OrderGateway {
    public:
        MPSCQueue<OrderRequest, 1024> req_queue;

        void send(const OrderRequest& req) {
            req_queue.push(req);
        }

        void run() {
            OrderRequest r;
            while (req_queue.pop(r)) {
                // 对接柜台 API
            }
        }
    };

}