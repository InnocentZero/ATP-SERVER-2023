#include "randomBot.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>

std::uniform_int_distribution<int> orderType(0, 1);
std::uniform_int_distribution<int> buySell(0, 1);
std::uniform_int_distribution<int> quantity(1, 10);
std::uniform_int_distribution<int> price(100, 200);

std::ostream &randomOrder(std::ostream &os) {
    std::default_random_engine rd(time(0));
    os << orderType(rd) << " " << buySell(rd) << " " << price(rd) << " "
       << quantity(rd) << std::endl;
    return os;
}
int main(int argc, char *argv[]) {
    std::string filename;
    if (argc > 1) {
        std::cerr << "Wrong usage. Specify filename for output or leave blank "
                     "for default filename.\n";
        return 0;
    } else if (argc == 1) {
        filename.assign(argv[0]);
    } else {
        filename.assign("output.txt");
    }
    std::ofstream output;
    output.open(filename);

    using namespace std::chrono;
    const duration<int64_t, std::micro> period = 5'000'000us;
    while (true) {
        randomOrder(output);
        std::this_thread::sleep_for(period);
    }

    return 0;
}
