//===-- ATP-SERVER/randomBot.hpp --===//
//
// Part of the ATP-SERVER Project
//===-----------------------------===//
///
/// \file
/// This is used to generate values for `filename` to take as input. An order
/// with randomized values is generated at regular intervals.
/// These outputs can be written into a file (“output.txt”) or taken by
/// `filename` directly
///
/// \brief
/// Usage:
/// \code
/// \endcode

/// Generates a sample order of
/// the format:   `N X P Q`
/// Where:
///  - N:
///     - 0 for market order
///     - 1 for limit order
///  - X:
///     - 0 for buying
///     - 1 for selling
///  - P: the price (100 < P < 200)
///  - Q  the quantity (1 < Q < 10)
#include <fstream>

std::ostream &randomOrder(std::ostream &os);
