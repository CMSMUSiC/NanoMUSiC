#ifndef ROOT_HELPERS
#define ROOT_HELPERS

#include <cmath>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "fmt/format.h"

#include "Math/QuantFuncMathCore.h"
#include "ROOT/RVec.hxx"
#include "TGraphAsymmErrors.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TH1F.h"
#include "TMath.h"

using namespace ROOT;
using namespace ROOT::VecOps;

namespace ROOTHelpers
{

template <typename H>
inline auto Print(const H &h) -> void
{
    std::cout << "Name: " << h->GetName() << " - Bins: " << h->GetNbinsX() << std::endl;
    std::cout << "Edges: [";
    std::cout << "-inf, ";
    for (int i = 1; i < h->GetNbinsX() + 2; i++)
    {
        std::cout << h->GetBinLowEdge(i);
        std::cout << ", ";
    }
    std::cout << "+inf";
    std::cout << "]" << std::endl;

    std::cout << "Counts: [";
    for (int i = 0; i < h->GetNbinsX() + 2; i++)
    {
        std::cout << h->GetBinContent(i);
        if (i < h->GetNbinsX() + 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;

    std::cout << "Errors: [";
    for (int i = 0; i < h->GetNbinsX() + 2; i++)
    {
        std::cout << h->GetBinError(i);
        if (i < h->GetNbinsX() + 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "]\n" << std::endl;
}

template <typename H>
inline auto Clone(const H &h, const std::optional<std::string> &new_name = std::nullopt) -> std::shared_ptr<TH1F>
{
    auto new_histo = std::make_shared<TH1F>(*h);
    if (new_name)
    {
        new_histo->SetName(new_name->c_str());
    }
    return new_histo;
}

template <typename H>
inline auto CloneAndReset(const H &h, const std::optional<std::string> &new_name = std::nullopt)
    -> std::shared_ptr<TH1F>
{
    auto new_histo = std::make_shared<TH1F>(*h);
    if (new_name)
    {
        new_histo->SetName(new_name->c_str());
    }
    new_histo->Reset();

    return new_histo;
}

template <typename H>
inline auto Counts(const H &h) -> RVec<double>
{
    auto counts = RVec<double>{};
    counts.reserve(h->GetNbinsX());

    for (int i = 1; i <= h->GetNbinsX(); i++)
    {
        counts.push_back(h->GetBinContent(i));
    }

    return counts;
}

template <typename H>
inline auto Errors(const H &h) -> RVec<double>
{
    auto errors = RVec<double>{};
    errors.reserve(h->GetNbinsX());

    for (int i = 1; i <= h->GetNbinsX(); i++)
    {
        errors.push_back(h->GetBinError(i));
    }

    return errors;
}

template <typename H>
inline auto Edges(const H &h) -> RVec<double>
{
    auto edges = RVec<double>{};
    edges.reserve(h->GetNbinsX() + 1);

    for (int i = 1; i <= h->GetNbinsX() + 1; i++)
    {
        edges.push_back(h->GetBinLowEdge(i));
    }

    return edges;
}

template <typename H>
inline auto Widths(const H &h) -> RVec<double>
{
    auto widths = RVec<double>{};
    widths.reserve(h->GetNbinsX());

    for (int i = 1; i <= h->GetNbinsX(); i++)
    {
        widths.push_back(h->GetBinWidth(i));
    }

    return widths;
}

template <typename H>
inline auto Centers(const H &h) -> RVec<double>
{
    auto centers = RVec<double>{};
    centers.reserve(h->GetNbinsX());

    for (int i = 1; i <= h->GetNbinsX(); i++)
    {
        centers.push_back(h->GetBinCenter(i));
    }

    return centers;
}

template <typename H>
inline auto Abs(const H &h) -> RVec<double>
{
    return ROOT::VecOps::abs(Counts(h));
}

template <typename H>
inline auto Pow2(const H &h) -> RVec<double>
{
    return ROOT::VecOps::pow(Counts(h), 2.);
}

template <typename H>
inline auto Sqrt(const H &h) -> RVec<double>
{
    return ROOT::VecOps::sqrt(Counts(h));
}

template <typename H>
inline auto Sum(const std::vector<H> &histos) -> RVec<double>
{
    if (histos.size() == 0)
    {
        fmt::print(stderr, "ERROR: Could not sum histograms. The provided list is empty.");
        std::exit(EXIT_FAILURE);
    }
    return std::transform_reduce(histos.cbegin(),
                                 histos.cend(),
                                 RVec<double>(histos[0]->GetNbinsX(), 0),
                                 std::plus{},
                                 [](const H &h) -> RVec<double>
                                 {
                                     return Counts(h);
                                 });
}

template <typename H>
inline auto SqrtSum(const std::vector<H> &histos) -> RVec<double>
{
    if (histos.size() == 0)
    {
        fmt::print(stderr, "ERROR: Could not square-sum histograms. The provided list is empty.");
        std::exit(EXIT_FAILURE);
    }
    return ROOT::VecOps::sqrt(std::transform_reduce(histos.cbegin(),
                                                    histos.cend(),
                                                    RVec<double>(histos[0]->GetNbinsX(), 0),
                                                    std::plus{},
                                                    [](const H &h) -> RVec<double>
                                                    {
                                                        return Pow2(h);
                                                    }));
}

///////////////////
/// returns |h1-h2|
template <typename H>
inline auto AbsDiff(const H &h1, const H &h2) -> RVec<double>
{
    return ROOT::VecOps::abs(Counts(h1) - Counts(h2));
}

/////////////////
/// This is important to sum Data and MC histograms and get the proper errors
template <typename H>
inline auto SumAsTH1F(const std::vector<H> &histos, const std::optional<std::string> &new_name = std::nullopt)
    -> std::shared_ptr<TH1F>
{
    if (histos.size() == 0)
    {
        fmt::print(stderr, "ERROR: Could not sum histograms. The provided list is empty.");
        std::exit(EXIT_FAILURE);
    }

    auto sum = Clone(histos[0]);
    for (std::size_t i = 1; i < histos.size(); i++)
    {
        sum->Add(histos[i].get());
    }

    if (new_name)
    {
        sum->SetName(new_name->c_str());
    }
    else
    {
        sum->SetName(histos[0].get()->GetName());
    }

    return sum;
}

///////////////////
/// Ref: https://twiki.cern.ch/twiki/bin/view/CMS/PoissonErrorBars
template <typename H>
inline auto MakeDataGraph(const H &data_histo, bool scale_to_area, double min_bin_width = 10.) -> TGraphAsymmErrors
{
    constexpr double alpha = 1 - 0.6827;
    auto scale_factor = 1.;
    TGraphAsymmErrors g = TGraphAsymmErrors(data_histo.get());

    for (int i = 0; i < g.GetN(); ++i)
    {
        if (scale_to_area)
        {
            scale_factor = data_histo->GetBinWidth(i + 1) / min_bin_width;
        }

        int N = static_cast<int>(std::round(data_histo->GetBinContent(i + 1) * scale_factor));
        double L = (N <= 0) ? 0 : (ROOT::Math::gamma_quantile(alpha / 2, N, 1.));

        // official recommendation
        double U = ROOT::Math::gamma_quantile_c(alpha / 2, N + 1, 1);
        // MUSiC does this ...
        // double U = static_cast<double>(N);

        g.SetPointEYlow(i, (N - L) / scale_factor);
        g.SetPointEYhigh(i, (U - N) / scale_factor);
    }

    return g;
}

///////////////////////////
/// Make the MC (background error band)
template <typename H>
inline auto MakeErrorBand(const H &h, const RVec<double> &uncertanties, bool scale_to_area, double min_bin_width = 10.)
    -> TGraphErrors
{
    if (static_cast<std::size_t>(h->GetNbinsX()) != uncertanties.size())
    {
        fmt::print(
            stderr,
            "ERROR: Could not create error band. The length of the uncertanties vector is not same as the number of "
            "bins.");
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
template <typename H>
inline auto MakeRatioGraph(const H &data_histo, const H &mc_histo) -> TGraphErrors
{
    if (data_histo->GetNbinsX() != mc_histo->GetNbinsX())
    {
        fmt::print(stderr,
                   "ERROR: Could not create ratio plot. The length of the Data and MC histograms are different.");
        std::exit(EXIT_FAILURE);
    }

    auto widths = Widths(data_histo);
    auto e_x = Widths(data_histo) / 2.;
    auto data_counts = Counts(data_histo);
    auto data_errors = Errors(data_histo);
    auto mc_counts = Counts(mc_histo);

    RVec<double> ratio;
    ratio.reserve(data_counts.size());
    RVec<double> ratio_err;
    ratio_err.reserve(mc_counts.size());

    for (std::size_t i = 0; i < data_counts.size(); i++)
    {
        if (mc_counts[i] == 0.)
        {
            ratio.push_back(-1);
            ratio_err.push_back(0);
        }
        else
        {
            ratio.push_back(data_counts[i] / mc_counts[i]);
            ratio_err.push_back(data_errors[i] / mc_counts[i]);
        }
    }

    return TGraphErrors(e_x.size(), Centers(data_histo).data(), ratio.data(), e_x.data(), ratio_err.data());
}

///////////////////////////
/// Make the MC ratio error (background error band)
template <typename H>
inline auto MakeRatioErrorBand(const H &mc_histo, const RVec<double> &uncertanties) -> TGraphErrors *
{
    if (mc_histo->GetNbinsX() != uncertanties.size())
    {
        fmt::print(
            stderr,
            "ERROR: Could not create ratio uncertanties bands. The length of the MC histograms and the uncertanties "
            "vector are different.");
        std::exit(EXIT_FAILURE);
    }

    auto widths = Widths(mc_histo);
    auto e_x = Widths(mc_histo) / 2.;
    auto mc_counts = Counts(mc_histo);

    RVec<double> ratio_mc(widths.size(), 1.);
    RVec<double> ratio_mc_err;
    ratio_mc_err.reserve(ratio_mc.size());

    for (std::size_t i = 0; i < ratio_mc.size(); i++)
    {
        if (mc_counts[i] == 0.)
        {
            ratio_mc_err.push_back(0);
        }
        else
        {
            ratio_mc_err.push_back(mc_counts[i] + uncertanties[i] / mc_counts[i]);
        }
    }

    return new TGraphErrors(e_x.size(), Centers(mc_histo).data(), ratio_mc.data(), e_x.data(), ratio_mc_err.data());
}

///////////////////////////
/// will convert a std::shared_ptr<TH1F> to TH1F*
/// usefull to pass to PyROOT
template <typename H>
inline auto GetRawPtr(const H &h) -> TH1F *
{
    return h.get();
}

} // namespace ROOTHelpers

namespace Uncertanties
{
template <typename H>
inline auto Symmetrize(const RVec<double> &v1, const RVec<double> &v2) -> RVec<double>
{
    return v1 / 2. + v2 / 2.;
}
} // namespace Uncertanties

#endif // ROOT_HELPERS