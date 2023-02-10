#ifndef GENERATOR_FILTERS

#include <functional>
#include <map>
#include <string>
#include <variant>

#include <fmt/format.h>

#include "ROOT/RVec.hxx"

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace std::string_literals;

namespace GeneratorFilters
{
using Inputs_t = std::vector<std::variant<std::string, float, double, bool, int, RVec<float>, RVec<int>>>;

inline auto dy_mass_filter(const Inputs_t &args) -> bool
{
    // fmt::print("Passei por aqui!\n");
    return true;
}

inline auto dy_pt_filter(const Inputs_t &args) -> bool
{
    return true;
}

const std::map<std::string, std::function<bool(Inputs_t)>> filters = {
    {"DYJetsToLL_M-50_13TeV_AM"s, dy_mass_filter}, //
};

} // namespace GeneratorFilters

#define GENERATOR_FILTERS
#endif // GENERATOR_FILTERS