#include "OrderBook.hpp"
#include <algorithm>
#include <limits>
namespace obook {

    std::vector<Trade> OrderBook::match(Order& incoming) {
        std::vector<Trade> newTrades;
        if (incoming.side == Side::Buy) {
            while (incoming.quantity > 0 && !_asks.empty()) {
                auto it = _asks.begin();
                if (incoming.priceBound < it->first) break;

                auto& queue = it->second;
                while (incoming.quantity > 0 && !queue.empty()) {
                    Order& resting = queue.front();
                    int fillQty = std::min(incoming.quantity, resting.quantity);

                    Trade t;
                    t.tradeId      = _nextTradeId;
                    t.buyOrderId   = incoming.id;
                    t.sellOrderId  = resting.id;
                    t.price        = resting.priceBound;
                    t.quantity     = fillQty;
                    t.timestamp    = Order::now();
                    newTrades.push_back(t);
                    _trades.push_back(t);
                    _nextTradeId++;

                    incoming.quantity -= fillQty;
                    resting.quantity  -= fillQty;

                    if (resting.quantity == 0) {
                        _orderIndex.erase(resting.id);
                        queue.pop_front();
                    }
                }
                if (queue.empty()) _asks.erase(it);
            }
        } else {
            while (incoming.quantity > 0 && !_bids.empty()) {
                auto rit = _bids.rbegin();
                if (incoming.priceBound > rit->first) break;

                auto& queue = rit->second;
                while (incoming.quantity > 0 && !queue.empty()) {
                    Order& resting = queue.front();
                    int fillQty = std::min(incoming.quantity, resting.quantity);

                    Trade t;
                    t.tradeId      = _nextTradeId;
                    t.buyOrderId   = resting.id;
                    t.sellOrderId  = incoming.id;
                    t.price        = resting.priceBound;
                    t.quantity     = fillQty;
                    t.timestamp    = Order::now();
                    newTrades.push_back(t);
                    _trades.push_back(t);
                    _nextTradeId++;

                    incoming.quantity -= fillQty;
                    resting.quantity  -= fillQty;

                    if (resting.quantity == 0) {
                        _orderIndex.erase(resting.id);
                        queue.pop_front();
                    }
                }
                if (queue.empty()) _bids.erase(std::prev(_bids.end()));
            }
        }
        return newTrades;
    }

    std::vector<Trade> OrderBook::addOrder(Order order) {
        if (order.type == OrderType::Market && order.priceBound < 0) {
            order.priceBound = (order.side == Side::Buy) ? std::numeric_limits<int>::max() : 0;
        }
        std::vector<Trade> trades = match(order);

        if (order.quantity > 0 && order.type == OrderType::Limit) {
            std::list<Order>::iterator itr;
            if (order.side == Side::Buy) {
                _bids[order.priceBound].push_back(order);
                itr = _bids[order.priceBound].end();
                itr--;
            } else {
                _asks[order.priceBound].push_back(order);
                itr = _asks[order.priceBound].end();
                itr--;
            }
            _orderIndex[order.id] = {order.side, order.priceBound, itr};
        }

        return trades;
    }

    bool OrderBook::cancelOrder(int orderId) {
    auto it = _orderIndex.find(orderId);
    if (it == _orderIndex.end()) return false;

    OrderLocation loc = it->second;
    auto& book = (loc.side == Side::Buy) ? _bids : _asks;

    auto orderPriceList = book.find(loc.price); 
    orderPriceList->second.erase(loc.itr);
    if (orderPriceList->second.empty()) {
        book.erase(orderPriceList);
    }

    _orderIndex.erase(it);
    return true;
}

    std::vector<PriceLevel> OrderBook::getBids(int n) const {
        std::vector<PriceLevel> result;
        for (auto it = _bids.rbegin(); it != _bids.rend() && (int)result.size() < n; ++it) {
            int total = 0;
            for (const auto& o : it->second) total += o.quantity;
            result.push_back({it->first, total});
        }
        return result;
    }

    std::vector<PriceLevel> OrderBook::getAsks(int n) const {
        std::vector<PriceLevel> result;
        for (auto it = _asks.begin(); it != _asks.end() && (int)result.size() < n; ++it) {
            int total = 0;
            for (const auto& o : it->second) total += o.quantity;
            result.push_back({it->first, total});
        }
        return result;
    }

    std::optional<int> OrderBook::spread() const {
        if (_bids.empty() || _asks.empty()) return std::nullopt;
        return _asks.begin()->first - _bids.rbegin()->first;
    }

    const std::vector<Trade>& OrderBook::getTrades() const {
        return _trades;
    }
}

