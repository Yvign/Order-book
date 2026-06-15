#include "OrderBook.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <atomic>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>

using namespace ftxui;
using namespace obook;

static std::string rjust(int w, int n) {
    std::ostringstream ss;
    ss << std::setw(w) << std::right << n;
    return ss.str();
}

static std::string relTime(int64_t us, int64_t t0) {
    int64_t d  = us - t0;
    int64_t s  = d / 1'000'000LL;
    int64_t ms = (d / 1'000LL) % 1'000LL;
    std::ostringstream ss;
    ss << '+' << s << '.' << std::setfill('0') << std::setw(3) << ms << 's';
    return ss.str();
}

static Element buildLadder(const OrderBook& book) {
    auto bids = book.getBids(5);
    auto asks = book.getAsks(5);
    auto sp   = book.spread();

    constexpr int W = 7;

    auto sideRows = [&](const std::vector<PriceLevel>& levels, bool isBid) {
        Elements rows;
        rows.push_back(hbox({
            text("  PRICE") | bold,
            text("    QTY") | bold,
            text("    CUM") | bold,
        }));
        rows.push_back(separator());
        int cum = 0;
        for (const auto& lvl : levels) {
            cum += lvl.totalQuantity;
            rows.push_back(hbox({
                text(rjust(W, lvl.price))         | color(isBid ? Color::Green : Color::Red),
                text(rjust(W, lvl.totalQuantity)),
                text(rjust(W, cum)),
            }));
        }
        return rows;
    };

    std::string spreadStr = sp ? "Spread: " + std::to_string(*sp) : "Spread: --";

    return window(
        text(" DEPTH LADDER "),
        vbox({
            hbox({
                vbox(sideRows(bids, true))  | flex,
                separator(),
                vbox(sideRows(asks, false)) | flex,
            }),
            separator(),
            text(spreadStr) | center,
        })
    );
}

static Element buildTape(const OrderBook& book, int64_t t0) {
    const auto& trades = book.getTrades();

    Elements rows;
    rows.push_back(hbox({
        text("     TIME") | bold,
        text("   PRICE") | bold,
        text("     QTY") | bold,
    }));
    rows.push_back(separator());

    int show  = std::min((int)trades.size(), 12);
    int start = (int)trades.size() - show;
    for (int i = (int)trades.size() - 1; i >= start; --i) {
        const auto& t = trades[i];
        rows.push_back(hbox({
            text(relTime(t.timestamp, t0)),
            text(rjust(8, t.price)),
            text(rjust(8, t.quantity)),
        }));
    }

    return window(
        text(" TRADE TAPE "),
        vbox(std::move(rows))
    );
}

int main() {
    OrderBook         book;
    std::mutex        mtx;
    std::atomic<bool> running{true};
    int64_t           t0 = Order::now();

    auto screen = ScreenInteractive::Fullscreen();

    auto ui = Renderer([&] {
        std::lock_guard<std::mutex> lock(mtx);
        return vbox({
            text(" obook — live order book ") | bold | center,
            separator(),
            hbox({
                buildLadder(book),
                buildTape(book, t0) | flex,
            }) | flex,
            text(" q  quit ") | dim | center,
        });
    });

    auto root = CatchEvent(ui, [&](Event e) {
        if (e == Event::Character('q')) {
            running = false;
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    std::thread sim([&] {
        using ms = std::chrono::milliseconds;
        struct Spec { int id; Side side; int price; int qty; OrderType type; };
        const std::vector<Spec> orders = {
            {1,  Side::Buy,  100, 30, OrderType::Limit},
            {2,  Side::Sell, 105, 20, OrderType::Limit},
            {3,  Side::Buy,   99, 15, OrderType::Limit},
            {4,  Side::Sell, 103, 10, OrderType::Limit},
            {5,  Side::Buy,   98, 25, OrderType::Limit},
            {6,  Side::Sell, 107,  8, OrderType::Limit},
            {7,  Side::Buy,   97, 12, OrderType::Limit},
            {8,  Side::Sell, 109,  6, OrderType::Limit},
            {9,  Side::Buy,  104, 18, OrderType::Limit},   
            {10, Side::Sell,  98, 22, OrderType::Limit}, 
            {11, Side::Buy,    0, 10, OrderType::Market},  
            {12, Side::Sell,   0,  8, OrderType::Market},  
        };

        for (const auto& o : orders) {
            if (!running) break;
            std::this_thread::sleep_for(ms(700));
            if (!running) break;
            {
                std::lock_guard<std::mutex> lk(mtx);
                book.addOrder({o.id, o.side, o.price, o.qty, Order::now(), o.type});
            }
            screen.PostEvent(Event::Custom);
        }
    });

    screen.Loop(root);
    running = false;
    sim.join();
    return 0;
}
