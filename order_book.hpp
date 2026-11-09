#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <map>
#include <algorithm>

enum class Side { Buy, Sell };

struct Order {
    uint64_t id;
    Side     side;
    int64_t  price;     // integer ticks, never double
    uint32_t quantity;
};

struct Trade {
    uint64_t buyId;
    uint64_t sellId;
    int64_t  price;     // price of the RESTING order
    uint32_t quantity;
};

class OrderBook {
public:
    // Matches the order against the opposite side (price-time priority).
    // Any unfilled remainder rests in the book. Returns the trades produced,
    // in the order they happened.
    std::vector<Trade> addOrder(Order order);

    // Removes a resting order. Returns false if the id is not in the book
    // (never existed, already filled, or already cancelled).
    bool cancelOrder(uint64_t id);

    std::optional<int64_t> bestBid() const;   // highest resting buy price
    std::optional<int64_t> bestAsk() const;   // lowest resting sell price

    // Total resting quantity at one price level (0 if none).
    uint32_t volumeAt(Side side, int64_t price) const;

private:
    std::map<int64_t, std::vector<Order>> bids_;
    std::map<int64_t, std::vector<Order>> asks_;

    // TODO: your data structures go here.
    // Hint: one container per side keyed by price, and a way to find an
    // order by id quickly so cancel does not have to scan everything.
};
