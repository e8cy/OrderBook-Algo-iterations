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
    int64_t  price;     // integer ticks doppeln sich nicht, sollte man klarstellen
    uint32_t quantity;
};

struct Trade {
    uint64_t buyId;
    uint64_t sellId;
    int64_t  price;
    uint32_t quantity;
};

class OrderBook {
public:
    std::vector<Trade> addOrder(Order order);
    bool cancelOrder(uint64_t id);

    std::optional<int64_t> bestBid() const;
    std::optional<int64_t> bestAsk() const;

    uint32_t volumeAt(Side side, int64_t price) const;

private:
    std::map<int64_t, std::vector<Order>> bids_;
    std::map<int64_t, std::vector<Order>> asks_;
};
