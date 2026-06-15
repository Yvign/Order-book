#pragma once

#include "Order.hpp"
#include "Trade.hpp"

#include <map>
#include <deque>
#include <vector>
#include <unordered_map>
#include <optional>

namespace obook {

    struct PriceLevel {
        int price;
        int totalQuantity;
    };

    struct OrderLocation {
        Side side;
        int  price;
    };

    class OrderBook {
        std::map<int, std::deque<Order>> _bids;
        std::map<int, std::deque<Order>> _asks;

        std::vector<Trade> _trades;
        int _nextTradeId = 1;

        std::map<int, OrderLocation> _orderIndex;

        std::vector<Trade> match(Order& incoming);
    public:
        std::vector<Trade>    addOrder(Order order);
        bool                  cancelOrder(int orderId);
        std::vector<PriceLevel> getBids(int n = 5) const;
        std::vector<PriceLevel> getAsks(int n = 5) const;
        std::optional<int>    spread() const;
        const std::vector<Trade>& getTrades() const;
    };
} 
