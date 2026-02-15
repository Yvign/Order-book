#pragma once

#include <cstdint>
#include <chrono>
#include <string>

namespace obook {

    enum class Side { Buy, Sell };
    enum class OrderType { Limit, Market };

    struct Order {
        int         id;
        Side        side;
        int         price;      
        int         quantity;
        int64_t     timestamp; 

        inline int64_t now() {
            using namespace std::chrono;
            return duration_cast<microseconds>(
                steady_clock::now().time_since_epoch()
            ).count();
        }
    };

    inline const char* sideToString(Side s) {
        return s == Side::Buy ? "BUY" : "SELL";
    }
}
