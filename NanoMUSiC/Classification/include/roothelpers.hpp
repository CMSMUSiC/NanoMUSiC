#ifndef ROOT_HELPERS
#define ROOT_HELPERS

#include <cmath>
#include <cstddef>
#include <fmt/core.h>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "Math/QuantFuncMathCore.h"
#include "ROOT/RVec.hxx"
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TH1.h"

using namespace ROOT;
using namespace ROOT::VecOps;

namespace ROOTHelpers
{

inline auto Print(const TH1F &h) -> void
{
    std::cout << "Name: " << h.GetName() << " - Bins: " << h.GetNbinsX() << std::endl;
    std::cout << "Edges: [";
    std::cout << "-inf, ";
    for (int i = 1; i < h.GetNbinsX() + 2; i++)
    {
        std::cout << h.GetBinLowEdge(i);
        std::cout << ", ";
    }
    std::cout << "+inf";
    std::cout << "]" << std::endl;

    std::cout << "Counts: [";
    for (int i = 0; i < h.GetNbinsX() + 2; i++)
    {
        std::cout << h.GetBinContent(i);
        if (i < h.GetNbinsX() + 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    std::cout << "Errors: [";
    for (int i = 0; i < h.GetNbinsX() + 2; i++)
    {
        std::cout << h.GetBinError(i);
        if (i < h.GetNbinsX() + 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]\n" << std::endl;
}

inline auto Clone(const TH1F &h, const std::optional<std::string> &new_name = std::nullopt) -> TH1F
{
    auto new_histo = static_cast<TH1F *>(h.Clone());
    if (new_name)
    {
        new_histo->SetName(new_name->c_str());
    }
    return *new_histo;
}

inline auto CloneAndReset(const TH1F &h, const std::optional<std::string> &new_name = std::nullopt) -> TH1F
{
    auto new_histo = static_cast<TH1F *>(h.Clone());
    if (new_name)
    {
        new_histo->SetName(new_name->c_str());
    }
    new_histo->Reset();

    return *new_histo;
}

inline auto Counts(const TH1F &h) -> RVec<double>
{
    auto counts = RVec<double>{};
    counts.reserve(h.GetNbinsX());

    for (int i = 1; i <= h.GetNbinsX(); i++)
    {
        counts.push_back(h.GetBinContent(i));
    }

    return counts;
}

inline auto Errors(const TH1F &h) -> RVec<double>
{
    auto errors = RVec<double>{};
    errors.reserve(h.GetNbinsX());

    for (int i = 1; i <= h.GetNbinsX(); i++)
    {
        errors.push_back(h.GetBinError(i));
    }

    return errors;
}

inline auto Edges(const TH1F &h) -> RVec<double>
{
    auto edges = RVec<double>{};
    edges.reserve(h.GetNbinsX() + 1);

    for (int i = 1; i <= h.GetNbinsX() + 1; i++)
    {
        edges.push_back(h.GetBinLowEdge(i));
    }

    return edges;
}

inline auto Widths(const TH1F &h) -> RVec<double>
{
    auto widths = RVec<double>{};
    widths.reserve(h.GetNbinsX());

    for (int i = 1; i <= h.GetNbinsX(); i++)
    {
        widths.push_back(h.GetBinWidth(i));
    }

    return widths;
}

inline auto Centers(const TH1F &h) -> RVec<double>
{
    auto centers = RVec<double>{};
    centers.reserve(h.GetNbinsX());

    for (int i = 1; i <= h.GetNbinsX(); i++)
    {
        centers.push_back(h.GetBinCenter(i));
    }

    return centers;
}

inline auto Abs(const TH1F &h) -> RVec<double>
{
    return ROOT::VecOps::abs(Counts(h));
}

inline auto Pow2(const TH1F &h) -> RVec<double>
{
    return ROOT::VecOps::pow(Counts(h), 2.);
}

inline auto Sqrt(const TH1F &h) -> RVec<double>
{
    return ROOT::VecOps::sqrt(Counts(h));
}

inline auto Sum(const std::vector<TH1F> &histos) -> RVec<double>
{
    if (histos.size() == 0)
    {
        fmt::print(stderr, "ERROR: Could not sum histograms. The provided list is empty.");
        std::exit(EXIT_FAILURE);
    }
    return std::transform_reduce(histos.cbegin(),
                                 histos.cend(),
                                 RVec<double>(histos[0].GetNbinsX(), 0),
                                 std::plus{},
                                 [](const TH1F &h) -> RVec<double>
                                 {
                                     return Counts(h);
                                 });
}

inline auto SqrtSum(const std::vector<TH1F> &histos) -> RVec<double>
{
    if (histos.size() == 0)
    {
        fmt::print(stderr, "ERROR: Could not square-sum histograms. The provided list is empty.");
        std::exit(EXIT_FAILURE);
    }
    return ROOT::VecOps::sqrt(std::transform_reduce(histos.cbegin(),
                                                    histos.cend(),
                                                    RVec<double>(histos[0].GetNbinsX(), 0),
                                                    std::plus{},
                                                    [](const TH1F &h) -> RVec<double>
                                                    {
                                                        return Pow2(h);
                                                    }));
}

///////////////////
/// returns |h1-h2|
inline auto AbsDiff(const TH1F &h1, const TH1F &h2) -> RVec<double>
{
    return ROOT::VecOps::abs(Counts(h1) - Counts(h2));
}

/////////////////
/// This is important to sum Data and MC histograms and get the proper errors
inline auto SumAsTH1F(const std::vector<TH1F> &histos, const std::optional<std::string> &new_name = std::nullopt)
    -> TH1F
{
    if (histos.size() == 0)
    {
        fmt::print(stderr, "ERROR: Could not sum histograms. The provided list is empty.");
        std::exit(EXIT_FAILURE);
    }

    auto sum = Clone(histos[0]);
    for (std::size_t i = 1; i < histos.size(); i++)
    {
        sum.Add(&histos[i]);
    }

    if (new_name)
    {
        sum.SetName(new_name->c_str());
    }
    else
    {
        sum.SetName(histos[0].GetName());
    }

    return sum;
}

inline auto SumAsTH1F(const std::vector<std::shared_ptr<TH1F>> &histos,
                      const std::optional<std::string> &new_name = std::nullopt) -> TH1F
{
    if (histos.size() == 0)
    {
        fmt::print(stderr, "ERROR: Could not sum histograms. The provided list is empty.");
        std::exit(EXIT_FAILURE);
    }

    auto sum = Clone(*histos[0]);
    for (std::size_t i = 1; i < histos.size(); i++)
    {
        sum.Add(histos[i].get());
    }

    if (new_name)
    {
        sum.SetName(new_name->c_str());
    }
    else
    {
        sum.SetName(histos[0]->GetName());
    }

    return sum;
}

inline auto SumAsTH1F(const std::unordered_map<std::string, TH1F> &histos,
                      bool remove_data = true,
                      const std::optional<std::string> &new_name = std::nullopt) -> TH1F
{
    auto values = std::vector<TH1F>();
    for (const auto &[pg, h] : histos)
    {
        if (remove_data)
        {
            if (pg == "Data")
            {
                continue;
            }
        }
        values.push_back(h);
    }
    return SumAsTH1F(values, new_name);
}

///////////////////
/// Ref: https://twiki.cern.ch/twiki/bin/view/CMS/PoissonErrorBastruct
struct PoissonError
{
    constexpr static double alpha = 1 - 0.6827;
    double low;
    double high;

    PoissonError(double val, double scale_factor = 1.)
    {
        int N = static_cast<int>(std::round(val));
        double L = (N <= 0) ? 0 : (ROOT::Math::gamma_quantile(alpha / 2, N, 1.));
        double U = ROOT::Math::gamma_quantile_c(alpha / 2, N + 1, 1);

        low = (N - L) * scale_factor;
        high = (U - N) * scale_factor;
    }
};

inline auto MakeDataGraph(const TH1F &data_histo,
                          bool scale_to_area,
                          std::pair<std::pair<int, double>, std::pair<int, double>> min_max,
                          double min_bin_width = 10.) -> TGraphAsymmErrors
{
    auto [_min, _max] = min_max;
    auto [idx_min, min] = _min;
    auto [idx_max, max] = _max;

    auto scale_factor = 1.;
    auto hist_clone = std::unique_ptr<TH1F>(static_cast<TH1F *>(data_histo.Clone()));
    if (scale_to_area)
    {
        hist_clone->Scale(min_bin_width, "width");
    }

    int n_graph_points = idx_max - idx_min + 1;
    // fmt::print("number of points: {}\n", n_graph_points);

    ROOT::VecOps::RVec<double> x;
    ROOT::VecOps::RVec<double> y;
    ROOT::VecOps::RVec<double> ex;
    ROOT::VecOps::RVec<double> ey_low;
    ROOT::VecOps::RVec<double> ey_high;
    x.reserve(n_graph_points);
    y.reserve(n_graph_points);
    ex.reserve(n_graph_points);
    ey_low.reserve(n_graph_points);
    ey_high.reserve(n_graph_points);

    for (int i = idx_min; i <= idx_max; ++i)
    {
        if (scale_to_area)
        {
            scale_factor = hist_clone->GetBinContent(i) / data_histo.GetBinContent(i);
        }

        // g.SetPoint(i - 1, hist_clone->GetBinCenter(i), hist_clone->GetBinContent(i));
        x.push_back(hist_clone->GetBinCenter(i));
        y.push_back(hist_clone->GetBinContent(i));
        // fmt::print("i: {} - {}\n", i, hist_clone->GetBinCenter(i));

        auto errors = PoissonError(data_histo.GetBinContent(i), scale_factor);
        // g.SetPointError(i - 1, 0., 0., errors.low, errors.high);
        ex.push_back(0.);
        ey_low.push_back(errors.low);
        ey_high.push_back(errors.high);
    }
    // fmt::print("Data Graph: {} - {} \n", idx_min, idx_max);
    auto g = TGraphAsymmErrors(x.size(), x.data(), y.data(), ex.data(), ex.data(), ey_low.data(), ey_high.data());
    // g.Print("all");
    return g;
}

inline auto GetMinMax(const TH1F &histogram_data, const TH1F &histogram_mc)
    -> std::pair<std::pair<int, double>, std::pair<int, double>>
{
    auto histogram = histogram_data;
    auto counts = Counts(histogram);

    // check for empty histogram
    if (ROOT::VecOps::Sum(counts) == 0)
    {
        histogram = histogram_mc;
        counts = Counts(histogram);
    }

    int first_nonzero_idx = 0;
    int last_nonzero_idx = std::numeric_limits<int>::max();

    for (std::size_t i = 0; i < counts.size(); i++)
    {
        if (first_nonzero_idx < 1 and counts[i] > 0)
        {
            first_nonzero_idx = i + 1;
        }
        if (counts[i] > 0)
        {
            last_nonzero_idx = i + 1;
        }
    }

    return {{first_nonzero_idx, histogram.GetBinLowEdge(first_nonzero_idx)},
            {last_nonzero_idx, histogram.GetBinLowEdge(last_nonzero_idx + 1)}};
}

inline auto GetYMin(const TH1F &histogram,
                    bool scale_to_area,
                    std::pair<std::pair<int, double>, std::pair<int, double>> min_max,
                    double min_bin_width = 10.) -> double
{
    auto [_min, _max] = min_max;
    auto [idx_min, min] = _min;
    auto [idx_max, max] = _max;

    auto y_min = std::numeric_limits<double>::max();
    auto histo_clone = std::unique_ptr<TH1F>(static_cast<TH1F *>(histogram.Clone()));

    if (scale_to_area)
    {
        histo_clone->Scale(min_bin_width, "width");
    }

    for (std::size_t i = static_cast<std::size_t>(idx_min); i <= static_cast<std::size_t>(idx_max); i++)
    {
        auto this_bin_content = histo_clone->GetBinContent(i);
        if (this_bin_content < y_min and this_bin_content > 0)
        {
            y_min = this_bin_content;
        }
    }

    if (y_min == std::numeric_limits<double>::max())
    {
        y_min = 1E-6;
    }

    return y_min;
}
inline auto GetYMax(const TH1F &histogram_data,
                    const TH1F &histogram_mc,
                    bool scale_to_area,
                    std::pair<std::pair<int, double>, std::pair<int, double>> min_max,
                    double min_bin_width = 10.) -> double
{
    auto [_min, _max] = min_max;
    auto [idx_min, min] = _min;
    auto [idx_max, max] = _max;

    auto y_max = 0.;
    auto data_histo_clone = std::unique_ptr<TH1F>(static_cast<TH1F *>(histogram_data.Clone()));
    auto mc_histo_clone = std::unique_ptr<TH1F>(static_cast<TH1F *>(histogram_mc.Clone()));

    if (scale_to_area)
    {
        data_histo_clone->Scale(min_bin_width, "width");
        mc_histo_clone->Scale(min_bin_width, "width");
    }

    for (std::size_t i = static_cast<std::size_t>(idx_min); i <= static_cast<std::size_t>(idx_max); i++)
    {
        auto mc_bin_content = mc_histo_clone->GetBinContent(i);
        auto data_bin_content = data_histo_clone->GetBinContent(i);
        if (mc_bin_content > y_max)
        {
            y_max = mc_bin_content;
        }
        if (data_bin_content + std::sqrt(data_bin_content) > y_max)
        {
            y_max = data_bin_content + std::sqrt(data_bin_content);
        }
    }

    if (y_max == 0.)
    {
        fmt::print(stderr, "ERROR: Could not set y_max for {} - {}.\n");
        exit(-1);
    }

    return y_max * 1.15;
}

///////////////////////////
/// Make the MC (background error band)
inline auto MakeErrorBand(const TH1F &h,
                          const RVec<double> &uncertanties,
                          bool scale_to_area,
                          double min_bin_width = 10.) -> TGraphErrors
{
    if (static_cast<std::size_t>(h.GetNbinsX()) != uncertanties.size())
    {
        fmt::print(stderr,
                   "ERROR: Could not create error band. The length of the uncertanties vector ({}) is not same as the "
                   "number of "
                   "bins ({}).\n",
                   uncertanties.size(),
                   h.GetNbinsX());
        std::exit(EXIT_FAILURE);
    }

    auto widths = Widths(h);
    auto e_x = Widths(h) / 2.;
    auto counts = Counts(h);
    auto scaled_uncertanties = uncertanties;
    if (scale_to_area)
    {
        counts = counts * min_bin_width / widths;
        scaled_uncertanties = scaled_uncertanties * min_bin_width / widths;
    }

    return TGraphErrors(e_x.size(), Centers(h).data(), counts.data(), e_x.data(), scaled_uncertanties.data());
}

///////////////////////////
/// Make the Data/MC ratio
inline auto MakeRatioGraph(const TH1F &data_histo,
                           const TH1F &mc_histo,
                           const RVec<double> &uncertainties,
                           std::pair<std::pair<int, double>, std::pair<int, double>> min_max)
    -> std::pair<TGraphAsymmErrors, TGraphErrors>
{
    auto [_min, _max] = min_max;
    auto [idx_min, min] = _min;
    auto [idx_max, max] = _max;

    if (data_histo.GetNbinsX() != mc_histo.GetNbinsX())
    {
        fmt::print(stderr,
                   "ERROR: Could not create ratio plot. The length of the Data and MC histograms are different.");
        std::exit(EXIT_FAILURE);
    }

    RVec<double> ratio;
    RVec<double> ratio_data_err_up;
    RVec<double> ratio_data_err_down;
    RVec<double> ratio_mc;
    RVec<double> ratio_mc_err;
    RVec<double> centers;
    RVec<double> erros_x;
    double accum_data = 0.;
    double accum_mc = 0.;
    double accum_mc_err_squared = 0.;
    double accum_bin_width = 0.;

    for (std::size_t i = static_cast<std::size_t>(idx_max); i >= static_cast<std::size_t>(idx_min); i--)
    {
        accum_data += data_histo.GetBinContent(i);
        accum_mc += mc_histo.GetBinContent(i);
        accum_mc_err_squared += std::pow(uncertainties[i - 1], 2.);
        accum_bin_width += mc_histo.GetBinWidth(i);
        if (accum_mc >= 0.1 or i == static_cast<std::size_t>(idx_min))
        {
            if (accum_mc > 0)
            {
                ratio.push_back(accum_data / accum_mc);

                // auto data_errors = PoissonError(accum_data);
                // ratio_data_err_up.push_back(data_errors.high / accum_mc);
                // ratio_data_err_down.push_back(data_errors.low / accum_mc);
                ratio_data_err_up.push_back(std::sqrt(accum_data) / accum_mc);
                ratio_data_err_down.push_back(std::sqrt(accum_data) / accum_mc);

                ratio_mc.push_back(1.);
                ratio_mc_err.push_back(std::sqrt(accum_mc_err_squared) / accum_mc);

                erros_x.push_back(accum_bin_width / 2.);
                centers.push_back(accum_bin_width / 2. + mc_histo.GetBinLowEdge(i));

                accum_data = 0.;
                accum_mc = 0.;
                accum_mc_err_squared = 0.;
                accum_bin_width = 0.;
            }
            else
            {
                ratio.push_back(-1.);
                ratio_data_err_up.push_back(0.);
                ratio_data_err_down.push_back(0.);

                ratio_mc.push_back(1.);
                ratio_mc_err.push_back(0.);

                erros_x.push_back(accum_bin_width / 2.);
                centers.push_back(accum_bin_width / 2. + mc_histo.GetBinLowEdge(i));

                accum_data = 0.;
                accum_mc = 0.;
                accum_mc_err_squared = 0.;
                accum_bin_width = 0.;
            }
        }
    }

    // reverse all vecs
    for (RVec<double> &vec : std::vector<std::reference_wrapper<RVec<double>>>{
             ratio, ratio_data_err_down, ratio_data_err_up, ratio_mc, ratio_mc_err, erros_x, centers})
    {
        vec = ROOT::VecOps::Reverse(vec);
    }

    return {TGraphAsymmErrors(ratio.size(),
                              centers.data(),
                              ratio.data(),
                              erros_x.data(),
                              erros_x.data(),
                              ratio_data_err_down.data(),
                              ratio_data_err_up.data()),
            TGraphErrors(ratio_mc.size(), centers.data(), ratio_mc.data(), erros_x.data(), ratio_mc_err.data())};
}

///////////////////////////
/// will convert a std::shared_ptr<TH1F> to TH1F*
/// usefull to pass to PyROOT
inline auto GetRawPtr(const std::shared_ptr<TH1F> &h) -> TH1F *
{
    return h.get();
}

} // namespace ROOTHelpers

namespace Uncertanties
{
// Helper functions
inline auto Symmetrize(const RVec<double> &v1, const RVec<double> &v2) -> RVec<double>
{
    return v1 / 2. + v2 / 2.;
}

// Integral Systematics: the same value for the whole sample
inline auto IntegralUncert(const TH1F &nom, const double scale) -> RVec<double>
{
    return ROOTHelpers::Counts(nom) * scale;
}

inline auto XSecOrder(const RVec<double> &total_mc_counts) -> RVec<double>
{
    return total_mc_counts * 0.;
}

// Constant Systematics: event-wise weight
inline auto AbsDiff(const TH1F &nom, const TH1F &shift) -> RVec<double>
{
    return ROOTHelpers::AbsDiff(nom, shift);
}

inline auto AbsDiffAndSymmetrize(const TH1F &nom, const TH1F &up, const TH1F &down) -> RVec<double>
{
    return Symmetrize(ROOTHelpers::AbsDiff(nom, up), ROOTHelpers::AbsDiff(nom, down));
}

} // namespace Uncertanties

#endif // ROOT_HELPERS
