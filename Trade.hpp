#pragma once

#include <cstdint>

namespace obook {
    struct Trade {
        int     tradeId;
        int     buyOrderId;
        int     sellOrderId;
        int     price;
        int     quantity;
        int64_t timestamp;
    };
}
