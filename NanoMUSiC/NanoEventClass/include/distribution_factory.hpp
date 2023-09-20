#ifndef DISTRIBUTION_FACOTRY_HPP_
#define DISTRIBUTION_FACOTRY_HPP_

#include "external/BS_thread_pool.hpp"
#include "fmt/format.h"

#include "Distribution.hpp"
#include "NanoEventClass.hpp"

auto distribution_factory(NanoEventClassCollection &ec_collection, bool counts_only)
    -> std::vector<std::shared_ptr<Distribution>>;

#endif // !DISTRIBUTION_FACOTRY_HPP_