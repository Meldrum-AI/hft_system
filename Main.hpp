#include "TDSystem/TradingSysType.h"
#include "LockFree/MsgQueue.hpp"
#include "OrderBook/OrderBook.hpp"
#include "Log/Logger.hpp"
#include <iostream>
#include <memory>

using namespace tdsys;

int main()
{
    // 1.初始化核心组件
    OrderBook orderBook;
    MsgQueue<SystemMessage> sysMsgQueue;

    // 2.模拟挂单
    Order* buyOrder = orderBook.create(Side::BUY, OrderType::LIMIT, 10000, 10);
    Order* sellOrder = orderBook.create(Side::SELL, OrderType::LIMIT, 10000, 10);

    orderBook.add(buyOrder);
    orderBook.add(sellOrder);

    // 3.撮合
    orderBook.match();

    std::cout << "[HFT] 底层支撑系统 + 订单簿 运行正常" << std::endl;

    return 0;
}
