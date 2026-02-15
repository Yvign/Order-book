#pragma once

#include "Order.hpp"
#include "Trade.hpp"

#include <map>
#include <deque>
#include <vector>

namespace obook {
    class OrderBook {
        std::map<int, std::deque<Order>> _bids;
        std::map<int, std::deque<Order>> _asks;

        std::vector<Trade> _trades;
        int _nextTradeId = 1;

        std::vector<Trade> match(Order& incoming);
    public:
        std::vector<Trade> addOrder(Order order);
    };
} 
