#include "OrderBook.hpp"
#include <iostream>

int main() {
    obook::OrderBook book;

    book.addOrder({1, obook::Side::Buy,  100, 10, obook::Order::now()});
    book.addOrder({2, obook::Side::Sell, 102,  5, obook::Order::now()});
    book.addOrder({3, obook::Side::Buy,   99, 20, obook::Order::now()});

    std::cout << "=== OrderBook Snapshot ===\n\n";

    std::cout << "Bids:\n";
    for (auto& level : book.getBids())
        std::cout << "  " << level.totalQuantity() << " @ " << level.price << "\n";

    std::cout << "\nAsks:\n";
    for (auto& level : book.getAsks())
        std::cout << "  " << level.price << " @ " << level.totalQuantity() << "\n";

    if (auto s = book.spread())
        std::cout << "\nSpread: " << *s << "\n";

    return 0;
}