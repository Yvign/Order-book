#include "OrderBook.hpp"
#include <algorithm>

namespace obook {

    std::vector<Trade> OrderBook::match(Order& incoming) {
        std::vector<Trade> newTrades;

        if(incoming.type == OrderType::Limit){
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
                            _orderIndex.erase(resting.id);
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
                            _orderIndex.erase(resting.id);
                            queue.pop_front();
                        }
                    }
                    if (queue.empty()) _bids.erase(std::prev(_bids.end()));
                }
            }

            return newTrades;
        } else {
            if (incoming.side == Side::Buy) {
                while (incoming.quantity > 0 && !_asks.empty()) {
                    auto it = _asks.begin();
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
                            _orderIndex.erase(resting.id);
                            queue.pop_front();
                        }
                    }
                    if (queue.empty()) _asks.erase(it);
                }
            } else {
                while (incoming.quantity > 0 && !_bids.empty()) {
                    auto rit = _bids.rbegin();
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
                            _orderIndex.erase(resting.id);
                            queue.pop_front();
                        }
                    }
                    if (queue.empty()) _bids.erase(std::prev(_bids.end()));
                }
            }
            return newTrades;
        }
    }

    std::vector<Trade> OrderBook::addOrder(Order order) {
        std::vector<Trade> trades = match(order);

        if (order.quantity > 0 && order.type == OrderType::Limit) {
            _orderIndex[order.id] = {order.side, order.price};

            if (order.side == Side::Buy) {
                _bids[order.price].push_back(order);
            } else {
                _asks[order.price].push_back(order);
            }
        }

        return trades;
    }

    bool OrderBook::cancelOrder(int orderId) {
        auto it = _orderIndex.find(orderId);
        if (it == _orderIndex.end()) return false;

        Side side  = it->second.side;
        int  price = it->second.price;

        auto& book  = (side == Side::Buy) ? _bids : _asks;
        auto  lvlIt = book.find(price);
        if (lvlIt == book.end()) return false;

        auto& queue = lvlIt->second;
        auto  pos   = std::find_if(queue.begin(), queue.end(),
                        [orderId](const Order& o) { return o.id == orderId; });

        if (pos == queue.end()) return false;

        queue.erase(pos);

        if (queue.empty()) {
            book.erase(lvlIt);
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

