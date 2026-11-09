#include "order_book.hpp"

#include <algorithm>
#include <iterator>

std::vector<Trade> OrderBook::addOrder(Order order) {
    std::vector<Trade> trades;

    if (order.side == Side::Buy) {
        while (order.quantity > 0 && !asks_.empty()) {
            auto askLevel = asks_.begin();
            if (askLevel->first > order.price) {
                break;
            }

            auto& restingOrders = askLevel->second;
            Order& restingSell = restingOrders.front();
            const uint32_t quantity = std::min(order.quantity, restingSell.quantity);

            trades.push_back(Trade{
                order.id,
                restingSell.id,
                restingSell.price,
                quantity
            });

            order.quantity -= quantity;
            restingSell.quantity -= quantity;

            if (restingSell.quantity == 0) {
                restingOrders.erase(restingOrders.begin());
            }
            if (restingOrders.empty()) {
                asks_.erase(askLevel);
            }
        }

        if (order.quantity > 0) {
            bids_[order.price].push_back(order);
        }
    } else {
        while (order.quantity > 0 && !bids_.empty()) {
            auto bidLevel = std::prev(bids_.end());
            if (bidLevel->first < order.price) {
                break;
            }

            auto& restingOrders = bidLevel->second;
            Order& restingBuy = restingOrders.front();
            const uint32_t quantity = std::min(order.quantity, restingBuy.quantity);

            trades.push_back(Trade{
                restingBuy.id,
                order.id,
                restingBuy.price,
                quantity
            });

            order.quantity -= quantity;
            restingBuy.quantity -= quantity;

            if (restingBuy.quantity == 0) {
                restingOrders.erase(restingOrders.begin());
            }
            if (restingOrders.empty()) {
                bids_.erase(bidLevel);
            }
        }

        if (order.quantity > 0) {
            asks_[order.price].push_back(order);
        }
    }

    return trades;
}

bool OrderBook::cancelOrder(uint64_t id) {
    // Search the buy side.
    for (auto level = bids_.begin(); level != bids_.end(); ++level) {
        auto& orders = level->second;
        auto it = std::find_if(orders.begin(), orders.end(),
                               [id](const Order& o) { return o.id == id; });
        if (it != orders.end()) {
            orders.erase(it);
            if (orders.empty()) {
                bids_.erase(level);   // remove the empty price level
            }
            return true;              // return right away, 'level' is now invalid
        }
    }

    // Search the sell side.
    for (auto level = asks_.begin(); level != asks_.end(); ++level) {
        auto& orders = level->second;
        auto it = std::find_if(orders.begin(), orders.end(),
                               [id](const Order& o) { return o.id == id; });
        if (it != orders.end()) {
            orders.erase(it);
            if (orders.empty()) {
                asks_.erase(level);
            }
            return true;
        }
    }

    return false;   // not in the book (never existed, filled, or already cancelled)
}


std::optional<int64_t> OrderBook::bestBid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    return bids_.rbegin()->first;
}

std::optional<int64_t> OrderBook::bestAsk() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    return asks_.begin()->first;
}

uint32_t OrderBook::volumeAt(Side side, int64_t price) const {
    const auto& sideOrders = (side == Side::Buy) ? bids_ : asks_;
    const auto level = sideOrders.find(price);
    if (level == sideOrders.end()) {
        return 0;
    }

    uint32_t volume = 0;
    for (const Order& order : level->second) {
        volume += order.quantity;
    }
    return volume;
}