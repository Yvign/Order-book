#include "OrderBook.hpp"
#include <algorithm>

namespace obook {

    std::vector<Trade> OrderBook::match(Order& incoming) {
        std::vector<Trade> newTrades;

        if (incoming.side == Side::Buy) {
            while (incoming.quantity > 0 && !_asks.empty()) {
                auto it = _asks.begin();
                if (incoming.price < it->first) break;

                auto& queue = it->second;
                while (incoming.quantity > 0 && !queue.empty()) {
                    Order& resting = queue.front();
                    int fillQty = std::min(incoming.quantity, resting.quantity);

                    Trade t;
                    t.tradeId      = _nextTradeId;
                    t.buyOrderId   = incoming.id;
                    t.sellOrderId  = resting.id;
                    t.price        = resting.price;
                    t.quantity     = fillQty;
                    t.timestamp    = Order::now();
                    newTrades.push_back(t);
                    _trades.push_back(t);
                    _nextTradeId++;

                    incoming.quantity -= fillQty;
                    resting.quantity  -= fillQty;

                    if (resting.quantity == 0) {
                        queue.pop_front();
                    }
                }
                if (queue.empty()) _asks.erase(it);
            }
        } else {
            while (incoming.quantity > 0 && !_bids.empty()) {
                auto rit = _bids.rbegin();
                if (incoming.price > rit->first) break;

                auto& queue = rit->second;
                while (incoming.quantity > 0 && !queue.empty()) {
                    Order& resting = queue.front();
                    int fillQty = std::min(incoming.quantity, resting.quantity);

                    Trade t;
                    t.tradeId      = _nextTradeId;
                    t.buyOrderId   = resting.id;
                    t.sellOrderId  = incoming.id;
                    t.price        = resting.price;
                    t.quantity     = fillQty;
                    t.timestamp    = Order::now();
                    newTrades.push_back(t);
                    _trades.push_back(t);
                    _nextTradeId++;

                    incoming.quantity -= fillQty;
                    resting.quantity  -= fillQty;

                    if (resting.quantity == 0) {
                        queue.pop_front();
                    }
                }
                if (queue.empty()) _bids.erase(std::prev(_bids.end()));
            }
        }

        return newTrades;
    }

    std::vector<Trade> OrderBook::addOrder(Order order) {
        std::vector<Trade> trades = match(order);

        if (order.quantity > 0) {
            if (order.side == Side::Buy) {
                _bids[order.price].push_back(order);
            } else {
                _asks[order.price].push_back(order);
            }
        }

        return trades;
    }
}
