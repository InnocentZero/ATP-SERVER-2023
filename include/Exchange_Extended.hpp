//===-- ATP-SERVER/Exchange_Extended.hpp --===//
//
// Part of the ATP-SERVER Project
//===-----------------------------===//
///
/// \file
/// Has Extended Functions and methods, along with a few bots
/// relevant to the \p Exchange class
//===-----------------------------===//
#ifndef EXCHANGE_EXTENDED_HPP
#define EXCHANGE_EXTENDED_HPP
#include "Exchange.hpp"
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include <fstream>
#include <mutex>
#include <queue>
#include <random>

typedef __gnu_pbds::tree<int, __gnu_pbds::null_type, std::less<int>,
                         __gnu_pbds::rb_tree_tag,
                         __gnu_pbds::tree_order_statistics_node_update>
    ord_set;

/// Generates a random number between 1 and \p r (both inclusive)
int64_t gr(int64_t r, std::mt19937 &rng);

/// Generates a random number between \p l and \p r (both inclusive)
int64_t glr(int64_t l, int64_t r, std::mt19937 &rng);

/// Processes and executes trading orders in a multi-threaded environment.
void Book_Updater(std::queue<Limit_Order> &pending_orders, Exchange &exchange__,
                  std::mutex &lck);

/// Simulates trading orders
void bot(std::mutex &m, Exchange &exchange__,
         std::queue<Limit_Order> &pending_orders, std::mt19937 &rng);

/// Computes the cumulative distribution function (CDF) of the standard normal
/// distribution
long double normalCDF(long double value);

/// returns the mean of a deque of numbers
long double calcMean(std::deque<long double> &v);

/// Returns the standard deviation of a deque of numbers
long double calcSd(std::deque<double> &v);

/// Simulates trade using normal distribution
void gaussianBot(Exchange &exchange__, std::queue<Limit_Order> &pending_orders,
                 std::mutex &lck);

/// Simple Moving Average (SMA) Bot. This bot uses the concept of moving
/// averages to make trading decisions based on the order book data.
void smaBot(Exchange &exchange__, std::queue<Limit_Order> &pending_orders,
            std::mutex &lck);

#endif
