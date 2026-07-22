# obook — Order Book & Matching Engine

A C++20 in-memory limit order book designed to process limit and market orders while
maintaining strict price-time priority (FIFO), with a live FTXUI terminal dashboard.

---

## 📌 Project Overview

An order book is the core component of modern financial exchanges — it tracks all active
buy (bid) and sell (ask) interest for an asset and discovers a market price by matching
overlapping orders. This project implements a deterministic matching engine that handles:

* **Order Ingestion** — accepting incoming limit and market orders.
* **Order Matching** — continuous matching of crossing buy/sell prices, price-time priority.
* **State Management** — tracking resting orders, partial fills, and cancellations.
* **Live Visualization** — a terminal UI showing top-of-book depth and a scrolling trade tape.

---

## 📷 Screenshot

<!-- Add your screenshot here, e.g.: -->
![obook terminal UI](screenshots/Pasted%20image.png)

---

## ⚙️ Core Architecture & Implementation

```
                ┌─────────────────────────┐
                │      Order Book         │
                └────────────┬────────────┘
                             │
       ┌─────────────────────┴─────────────────────┐
       ▼                                           ▼
┌───────────────┐                           ┌───────────────┐
│ Bids (map<int, │                          │ Asks (map<int, │
│  list<Order>>) │                          │  list<Order>>) │
└───────┬───────┘                           └───────┬───────┘
        │  sorted descending (best = last)          │  sorted ascending (best = first)
        ▼                                           ▼
┌───────────────┐                           ┌───────────────┐
│ Price Level   │ [Ord1] <-> [Ord2]         │ Price Level   │ [Ord3] <-> [Ord4]
│ (std::list)   │  FIFO within level        │ (std::list)   │  FIFO within level
└───────────────┘                           └───────────────┘

        _orderIndex: unordered_map<OrderID, {side, price, list::iterator}>
                     → direct handle into the exact resting order node
```

### Data Structures Used

1. **Price Map (`std::map<int, std::list<Order>>`)**
   * Bids and asks are each a separate ordered map, keyed by price.
   * Bids: best price is the *last* element (`_bids.rbegin()`). Asks: best price is the
     *first* element (`_asks.begin()`).
   * Insertion/removal of a price level is O(log P), where P is the number of distinct
     price levels currently in the book. Walking consecutive levels (as the depth ladder
     does) is O(1) amortized per step, since it's plain iterator traversal, not a keyed
     lookup.

2. **Price Level Queue (`std::list<Order>`)**
   * Each price level holds a doubly linked list of resting orders in arrival order.
   * Enforces **FIFO (price-time priority)**: earlier orders at a price fill first.
   * Erasing a specific order (given an iterator to it) is O(1) and does not invalidate
     iterators to any other order in the same or a different list.

3. **Order Index (`std::unordered_map<int, OrderLocation>`)**
   * Maps `OrderID → {side, price, iterator}`.
   * The stored iterator points directly at the order's node inside the relevant price
     level's `list`, giving O(1) average-case cancellation — no scan of the price level
     required.

---

## 🔄 Matching Algorithm Logic

### 1. Limit Order Processing
* **Crossing the book:** a Buy Limit order matches while its price ≥ the best ask; a Sell
  Limit order matches while its price ≤ the best bid.
* **Partial vs. full fills:** if incoming quantity exceeds the resting order's quantity,
  the resting order is fully consumed and removed, and matching continues against the
  next order/price level. If incoming quantity is smaller, the resting order's quantity
  is decremented and matching stops.
* **Resting order:** any unmatched remainder is appended to the tail of the corresponding
  price level's list.

### 2. Market Order Processing (IOC-style price bound)
* Market orders carry a `priceBound`. A **non-negative** value bounds execution —
  "don't buy above X" / "don't sell below X" — behaving like an Immediate-Or-Cancel (IOC)
  limit order. A **negative** value means unbounded: sweep the book at any price until
  filled or liquidity is exhausted.
* Internally, a negative `priceBound` is translated in `addOrder` to a side-correct
  sentinel (`INT_MAX` for buys, `INT_MIN` for sells) before matching runs, so the same
  comparison logic in `match()` correctly handles both bounded and unbounded orders.
* Market orders never rest: any unfilled remainder beyond the price bound or available
  liquidity is dropped, not inserted into the book.

### 3. Order Cancellation
* Look up `OrderID` in `_orderIndex` — O(1) average.
* Erase the order directly via its stored `list` iterator — O(1), no scan.
* If the price level's list becomes empty as a result, the price level itself is removed
  from `_bids`/`_asks`, matching the cleanup `match()` already performs when a level
  drains during matching.

---

## 🛠️ Code Structure

```
Order.hpp       — Order struct, Side/OrderType enums, timestamp helper
Trade.hpp       — Trade struct (executed fill record)
OrderBook.hpp   — OrderBook class interface, PriceLevel / OrderLocation structs
OrderBook.cpp   — matching engine implementation
main.cpp        — FTXUI terminal dashboard + simulated order feed
CMakeLists.txt  — build configuration, fetches FTXUI v5.0.0 via FetchContent
```

---

## 🚀 How to Run

### Prerequisites
* **CMake ≥ 3.14**
* **C++20** compiler (GCC ≥ 10 / Clang ≥ 10)
* Internet access on first configure (FTXUI is fetched automatically via `FetchContent`)

### Build & Run

```bash
git clone https://github.com/Yvign/Order-book.git
cd Order-book

mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

./obook
```

Requires a real terminal. On launch, a background thread feeds a fixed pre-set number of
orders into the book, one every 700ms, so you can watch the depth ladder and trade tape
update live. Press **`q`** to quit.
