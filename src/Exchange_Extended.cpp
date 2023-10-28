#include "Exchange_Extended.hpp"
#include "Exchange.hpp"

const std::int16_t ENDTIME = 20000;

int64_t gr(int64_t r, std::mt19937 &rng) {
    std::uniform_int_distribution<int> uid(0, r - 1);
    int val = uid(rng);
    return val + 1;
}

int64_t glr(int64_t l, int64_t r, std::mt19937 &rng) {
    int64_t dif = r - l + 1;
    int64_t x = gr(dif, rng) + l - 1;
    return x;
}

void Book_Updater(std::queue<Limit_Order> &pending_orders, Exchange &exchange__,
                  std::mutex &lck) {
    while (true) {
        if (lck.try_lock()) {
            if (!pending_orders.empty()) {
                auto order = pending_orders.front();
                pending_orders.pop();
                bool buy = order.buy;
                long double price = order.price;
                long long quantity = order.quantity;
                t_point time = order.time;
                if (order.price == -2) {
                    exchange__.Fill_Market_Order(buy, quantity);
                } else {
                    exchange__.Add_Limit_Order(buy, price, quantity, time);
                }
            }
            lck.unlock();
        }
        // this_thread::sleep_for(chrono::milliseconds(1));
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
}

void bot1(std::mutex &m, Exchange &exchange__,
          std::queue<Limit_Order> &pending_orders, std::mt19937 &rng) {
    bool bot1_flag = true;
    while (bot1_flag) {
        if (m.try_lock()) {
            long double price;
            if (exchange__.bid_price == -1) {
                price = std::min(glr(3000, 6500, rng),
                                 exchange__.ask_price - glr(1, 100, rng));
            } else {
                price =
                    glr(1, 301, rng) + exchange__.bid_price - 151 +
                    glr(1, Get_Time().time_since_epoch().count() / 6 + 1, rng);
            }
            long long quantity = gr(100, rng);
            std::ofstream fout("Rand_orders.txt", std::ios_base::app);
            fout << Get_Time().time_since_epoch().count() << "," << 1 << ","
                 << price << "," << quantity << "\n";
            pending_orders.push(Limit_Order(true, price, quantity, Get_Time()));

            if (exchange__.ask_price == 1e15) {
                price = std::max(
                    static_cast<long double>(glr(
                        3000, 6500 + Get_Time().time_since_epoch().count() / 2,
                        rng)),
                    static_cast<long double>(exchange__.bid_price +
                                             glr(1, 100, rng)));
            } else {
                price =
                    exchange__.ask_price + 151 - glr(1, 301, rng) +
                    glr(1, Get_Time().time_since_epoch().count() / 6 + 1, rng);
            }
            quantity = gr(100, rng);
            fout << Get_Time().time_since_epoch().count() << "," << 0 << ","
                 << price << "," << quantity << "\n";
            fout.close();
            pending_orders.push(
                Limit_Order(false, price, quantity, Get_Time()));
            m.unlock();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(3000));

        if (Get_Time().time_since_epoch().count() > ENDTIME) {
            bot1_flag = false;
        }
    }
}

std::default_random_engine rd(time(0));
std::uniform_int_distribution<int> quantityDist(1, 10);
std::uniform_real_distribution<float> pickDist(0, 1);

long double normalCDF(long double value) {
    return 0.5 * erfc(-value * sqrtl(0.5));
}

long double calcMean(std::deque<long double> &v) {
    long double sum = 0;
    for (std::size_t iter = 0; iter < v.size(); iter++) {
        sum += v[iter];
    }
    sum /= v.size();
    return sum;
}

long double calcSd(std::deque<long double> &v) {
    long double sd = 0;
    auto mean = calcMean(v);
    for (std::size_t iter = 0; iter < v.size(); iter++) {
        sd += (v[iter] - mean) * (v[iter] - mean);
    }
    sd /= v.size();
    sd = sqrtl(sd);
    return sd;
}

void gaussianBot(Exchange &exchange__, std::queue<Limit_Order> &pending_orders,
                 std::mutex &lck) {
    long double prevBids = -1, prevAsks = -1;
    int64_t n = 5; // n is the size of the window
    std::deque<long double> mktBids(n);
    std::deque<long double> mktAsks(n);
    iota(mktAsks.begin(), mktAsks.end(), 100);
    iota(mktBids.begin(), mktBids.end(), 100);
    while (Get_Time().time_since_epoch().count() < ENDTIME) {
        if (exchange__.ask_price != prevAsks) {
            auto z =
                (exchange__.ask_price - calcMean(mktAsks)) / calcSd(mktAsks);
            if (z < 0) {
                z = -z;
            }
            auto acceptProbability = normalCDF(z) + 0.5;
            auto p = pickDist(rd);
            if (p <= acceptProbability) { // accept the ask
                auto price = exchange__.ask_price;
                if (price == 1e15) {
                    // I do not know why this is necessary but
                    // it is, if removed the code doesn't work
                    price /= 1.000000000001;
                }
                if (lck.try_lock()) {
                    std::ofstream fout("Gauss_orders.txt", std::ios_base::app);
                    fout << Get_Time().time_since_epoch().count() << "," << 1
                         << "," << price << "," << quantityDist(rd) << "\n";
                    pending_orders.push(
                        Limit_Order(true, price, quantityDist(rd), Get_Time()));
                    fout.close();
                    lck.unlock();
                }
            }
            prevAsks = exchange__.ask_price;
            mktAsks.push_back(exchange__.ask_price);
            mktAsks.pop_front();
        }
        if (exchange__.bid_price != prevBids) {
            auto z =
                (exchange__.bid_price - calcMean(mktBids)) / calcSd(mktBids);
            if (z < 0) {
                z = -z;
            }
            auto acceptProbability = normalCDF(z) + 0.5;
            auto p = pickDist(rd);
            if (p <= acceptProbability) {
                auto price = exchange__.ask_price;
                if (price == 1e15) {
                    price /= 8e12;
                }
                if (lck.try_lock()) {
                    std::ofstream fout("Gauss_orders.txt", std::ios_base::app);
                    pending_orders.push(Limit_Order(
                        false, price, quantityDist(rd), Get_Time()));
                    fout << Get_Time().time_since_epoch().count() << "," << 0
                         << "," << price << "," << quantityDist(rd) << "\n";
                    fout.close();
                    lck.unlock();
                }
            }
            prevBids = exchange__.bid_price;
            mktBids.push_back(exchange__.bid_price);
            mktBids.pop_front();
        }
    }
}

void smaBot(Exchange &exchange__, std::queue<Limit_Order> &pending_orders,
            std::mutex &lck) {
    long double prevAsks = -1;
    std::deque<long double> mktAsks20(20), mktAsks5(5);
    iota(mktAsks20.begin(), mktAsks20.end(), 100);
    iota(mktAsks5.begin(), mktAsks5.end(), 100);

    while (Get_Time().time_since_epoch().count() < ENDTIME) {
        if (exchange__.ask_price != prevAsks) {
            mktAsks20.push_back(exchange__.ask_price);
            mktAsks20.pop_front();
            mktAsks5.push_back(exchange__.ask_price);
            mktAsks5.pop_front();
            prevAsks = exchange__.ask_price;
            double meanAsks20 = calcMean(mktAsks20),
                   meanAsks5 = calcMean(mktAsks5);

            if (meanAsks5 > meanAsks20) {
                auto price = exchange__.ask_price;
                if (price == 1e15) {
                    price /= 1.000000000001;
                }
                if (lck.try_lock()) {
                    std::ofstream fout("SMA_orders.txt", std::ios_base::app);
                    pending_orders.push(
                        Limit_Order(true, price, quantityDist(rd), Get_Time()));
                    fout << Get_Time().time_since_epoch().count() << "," << 1
                         << "," << price << "," << quantityDist(rd) << "\n";
                    fout.close();
                    lck.unlock();
                }
            }

            if (meanAsks5 < meanAsks20) { // accept the bid
                auto price = exchange__.ask_price;
                if (price == 1e15) { // I do not know why this is necessary
                    price /= 8e12;
                }
                if (lck.try_lock()) {
                    std::ofstream fout("SMA_orders.txt", std::ios_base::app);
                    pending_orders.push(Limit_Order(
                        false, price, quantityDist(rd), Get_Time()));
                    fout << Get_Time().time_since_epoch().count() << "," << 0
                         << "," << price << "," << quantityDist(rd) << "\n";
                    fout.close();
                    lck.unlock();
                }
            }
        }
    }
}

// int main() {
//     Start_Exchange();
//
//     std::thread t1(Book_Updater);
//     t1.detach();
//
//     std::thread t2(Broadcaster);
//     t2.detach();
//
//     std::thread t3(bot1);
//     t3.detach();
//
//     std::thread t4(gaussianBot);
//     t4.detach();
//
//     std::thread t5(smaBot);
//     t5.detach();
//
//     queue<Limit_Order> Manual_orders;
//
//     while (true) {
//         short int order_type;
//         std::cin >> order_type;
//         bool buy;
//         long double price;
//         long long quantity;
//         if (order_type == 0) {
//             std::cin >> buy >> price >> quantity;
//             // _exchange.Add_Limit_Order(buy, price, quantity, Get_Time());
//             if (lck.try_lock()) {
//                 std::ofstream fout("Manual_orders.txt", std::ios_base::app);
//                 while (!Manual_orders.empty()) {
//                     Limit_Order temp = Manual_orders.front();
//                     pending_orders.push(Manual_orders.front());
//                     Manual_orders.pop();
//                     fout << temp.time << "," << temp.buy << "," << temp.price
//                          << "," << temp.quantity << "\n";
//                 }
//                 pending_orders.push(
//                     Limit_Order(buy, price, quantity, Get_Time()));
//                 fout << Get_Time() << "," << buy << "," << price << ","
//                      << quantity << "\n";
//                 fout.close();
//                 lck.unlock();
//             } else {
//                 Manual_orders.push(
//                     Limit_Order(buy, price, quantity, Get_Time()));
//             }
//             // pending_orders.push(Limit_Order(buy, price, quantity,
//             // Get_Time()));
//         } else if (order_type == 1) {
//             std::cin >> buy >> quantity;
//             //_exchange.Fill_Market_Order(buy, quantity);
//             if (lck.try_lock()) {
//                 std::ofstream fout("Manual_orders.txt", std::ios_base::app);
//                 while (!Manual_orders.empty()) {
//                     Limit_Order temp = Manual_orders.front();
//                     pending_orders.push(Manual_orders.front());
//                     Manual_orders.pop();
//                     fout << temp.time << "," << temp.buy << "," << temp.price
//                          << "," << temp.quantity << "\n";
//                 }
//                 pending_orders.push(Limit_Order(buy, -2, quantity,
//                 Get_Time())); fout << Get_Time() << "," << buy << "," << -2
//                 << "," << quantity
//                      << "\n";
//                 fout.close();
//                 lck.unlock();
//             } else {
//                 Manual_orders.push(Limit_Order(buy, -2, quantity,
//                 Get_Time()));
//             }
//             // pending_orders.push(Limit_Order(buy, -1, quantity,
//             Get_Time()));
//         } else {
//             break;
//         }
//     }
//     // wait for book updater to finish
//     while (bot1_flag) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
//
//     while (!pending_orders.empty()) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
//     while (!broadcast_queue.empty()) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
//
//     return 0;
// }
