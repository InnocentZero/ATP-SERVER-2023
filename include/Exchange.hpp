//===-- ATP-SERVER/Exchange.hpp --===//
//
// Part of the ATP-SERVER Project
//===-----------------------------===//
///
/// \file
/// This file contains the declarations
/// of the Exchange class and the
/// related Limit_Order struct
///
//===-----------------------------===//

#ifndef EXCHANGE_HPP
#define EXCHANGE_HPP

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

/// \brief
/// Follows a FIFO Exchange
///
/// Input order Format:
/// \code
///     0 X P Q
///     1 X Q
/// \endcode
///
/// Parameters:
/// - 0 : Limit Order
/// - 1 : Market Order
/// - X : 0/1
/// - 0 : Sell
/// - 1 : Buy
/// - P : Price
/// - Q : Quantity
///
///
///
/// If market order is not filled, prints "Market order quantity is more than
/// available quantity" and trades with available quantity.
///
/// Input -1 to end the exchange

// global scope typedef for maintaining time durations
typedef int64_t time_dur;

/// Sets global timestamp to 0;
void Start_Clock();

/// Returns the current time since the clock was started.
time_dur Get_Time();

/// Stores a limit order
///
/// If \p buy is true, higher \p price is given higher priority. If \p price is
/// equal, then lower \p time is given higher priority. If \p buy is false,
/// lower \p price is given higher priority, If \p price is equal, then lower \p
/// time is given higher priority.
struct Limit_Order {
    bool buy; // true -> buy, false -> sell
    int64_t price, quantity;
    time_dur time;
    Limit_Order(bool buy, int64_t price, int64_t quantity, time_dur time);

    // Comparator
    bool operator<(const Limit_Order &other) const;
};

/// Stores an exchange
///
/// \p buy_orders keeps a list of th buy orders. Similarly, \p sell_orders keeps
/// a list of sell orders.
///
/// \p ask_prices maintains a count of the number of
/// elements available at the given \p ask_price.
/// \p bid_prices maintains a count of the number of elements available at the
/// given \p bid_price
class Exchange {
  private:
    std::set<Limit_Order> buy_orders, sell_orders;
    int64_t ask_price, ask_quantity, bid_price, bid_quantity,
        total_quantity_ask, total_quantity_bid;
    std::map<int64_t, int64_t> ask_prices, bid_prices;

    /// Sets \p bid_price and \p ask_price for a particular exchange.
    void Update_Market_Values();

    /// Broadcast Format:
    /// T AP AQ BP BQ FP FQ
    ///
    /// Parameters:
    /// - T -> Time in milliseconds from the start of the exchange
    /// - AP -> Ask Price
    /// - AQ -> Ask Quantity
    /// - BP -> Bid Price
    /// - BQ -> Bid Quantity
    /// - FP -> Fill Price
    /// - FQ -> Fill Quantity
    ///
    /// If there are no fill, FP and FQ are left empty.
    /// If there are multiple fills, multiple lines are printed.
    void Broadcast(int64_t fill_price, int64_t fill_quantity);
    void Broadcast();

  public:
    /// Satisfies market orders from the current pool of orders present.
    ///
    /// \param buy kind of order. If `true`, it is a buy order, else a sell
    /// order.
    void Fill_Market_Order(bool buy, int64_t quantity);

    /// Adds a limit order and attempts to match it immediately.
    ///
    /// \param buy kind of order. If `true`, it is a buy order, else a sell
    /// order.
    void Add_Limit_Order(bool buy, int64_t price, int64_t quantity,
                         int64_t time);

    void Match(bool buy, int64_t price, int64_t quantity, int64_t time);

    Exchange();
    Exchange(Exchange &&) = default;
    Exchange(const Exchange &) = default;
    Exchange &operator=(Exchange &&) = default;
    Exchange &operator=(const Exchange &) = default;
    ~Exchange();
};

#endif
