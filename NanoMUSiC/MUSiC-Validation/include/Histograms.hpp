#ifndef HISTOGRAMS
#define HISTOGRAMS

#include "BinLimits.hpp"
#include "Configs.hpp"
#include <TH1F.h>
#include <cmath>
#include <math.h>
#include <vector>

#include "fmt/format.h"

// create a new histogram with fixed bin size
#define ADD_TH1F(HISTO, N_BINS, LOWER_BOUND, UPPER_BOUND)                                                              \
    TH1F HISTO = TH1F(#HISTO, #HISTO, N_BINS, LOWER_BOUND, UPPER_BOUND)

// define a new histogram, with variable bin size
inline auto rebin_histogram(                             //
    TH1F &hist,                                          //
    const std::map<std::string, int> &countMap,          //
    bool is_Z_mass_validation = false,                   //
    const std::string &distribution = "validation_plot", //
    double min = 0.,                                     //
    double max = 13000.,                                 //
    double fudge = 1.,                                   //
    double min_bin_size = 10.                            //
    ) -> TH1F
{
    if (is_Z_mass_validation)
    {
        min = PDG::Z::Mass - 20.;
        max = PDG::Z::Mass + 20.;
        min_bin_size = 1.;
    }

    auto limits = BinLimits::get_bin_limits(distribution, countMap, min, max, min_bin_size, fudge);

    return TH1F(hist.GetName(), hist.GetTitle(), limits.size() - 1 /*#bins = #edges-1*/, limits.data());
}

constexpr int n_energy_bins = 1300;
constexpr float min_energy = 0;
constexpr float max_energy = 13000;

constexpr int n_eta_bins = 20;
constexpr float min_eta = -3.;
constexpr float max_eta = 3.;

constexpr int n_phi_bins = 20;
constexpr float min_phi = -M_PI;
constexpr float max_phi = M_PI;

constexpr float min_dR = 0;
constexpr float max_dR = 10;
constexpr int n_dR_bins = static_cast<int>((max_dR - min_dR) / 0.4);

constexpr int n_multiplicity_bins = 11;
constexpr float min_multiplicity = -0.5;
constexpr float max_multiplicity = static_cast<float>(n_multiplicity_bins - 1) + 0.5;

#endif // !HISTOGRAMS