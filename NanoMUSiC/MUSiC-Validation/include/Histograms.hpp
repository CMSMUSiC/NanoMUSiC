#ifndef HISTOGRAMS
#define HISTOGRAMS

#include <TH1F.h>
#include <cmath>
#include <math.h>
#include <vector>

#define ADD_TH1F(HISTO, N_BINS, LOWER_BOUND, UPPER_BOUND)                                                              \
    TH1F HISTO = TH1F(#HISTO, #HISTO, N_BINS, LOWER_BOUND, UPPER_BOUND)

constexpr int n_energy_bins = 1300;
constexpr float min_energy = 0;
constexpr float max_energy = 13000;

constexpr int n_eta_bins = 20;
constexpr float min_eta = -3.;
constexpr float max_eta = 3.;

constexpr int n_phi_bins = 20;
constexpr float min_phi = -M_PI;
constexpr float max_phi = M_PI;

constexpr int n_dR_bins = 10;
constexpr float min_dR = 0;
constexpr float max_dR = 10;

constexpr int n_multiplicity_bins = 11;
constexpr float min_multiplicity = -0.5;
constexpr float max_multiplicity = static_cast<float>(n_multiplicity_bins - 1) + 0.5;

#endif // !HISTOGRAMS