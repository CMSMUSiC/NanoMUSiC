#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "TH2.h"
// #include "fmt/format.h"
#include <memory>
#include <optional>

auto compute_btag_efficiency(const std::string &sample,
                             const std::string &process_group,
                             const std::optional<std::string> &generator_filter,
                             const std::string &input_file,
                             const std::string &year) -> double
{
    // create btag efficiency histograms
    constexpr std::array<double, 12> pt_bins = {std::numeric_limits<double>::lowest(),
                                                20.,
                                                30.,
                                                50.,
                                                70.,
                                                100.,
                                                140.,
                                                200.,
                                                300.,
                                                600.,
                                                1000.,
                                                std::numeric_limits<double>::max()};
    // auto btag_efficiency_light_num =
    //     TH2D(fmt::format("[{}]_light_num", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4,
    //     0., 3.);
    // auto btag_efficiency_light_den =
    //     TH2D(fmt::format("[{}]_light_den", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4,
    //     0., 3.);
    // auto btag_efficiency_c_num =
    //     TH2D(fmt::format("[{}]_c_num", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);
    // auto btag_efficiency_c_den =
    //     TH2D(fmt::format("[{}]_c_den", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);
    // auto btag_efficiency_b_num =
    //     TH2D(fmt::format("[{}]_b_num", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);
    // auto btag_efficiency_b_den =
    //     TH2D(fmt::format("[{}]_b_den", process_group).c_str(), "", pt_bins.size() - 1, pt_bins.data(), 4, 0., 3.);
    //
    // btag_efficiency_light_num.Sumw2();
    // btag_efficiency_light_den.Sumw2();
    // btag_efficiency_c_num.Sumw2();
    // btag_efficiency_c_den.Sumw2();
    // btag_efficiency_b_num.Sumw2();
    // btag_efficiency_b_den.Sumw2();

    ROOT::RDataFrame df("Events", input_file);

    auto colNames = df.GetColumnNames();
    auto has_genWeight = false;
    auto has_LHEWeight_originalXWGTUP = false;
    for (auto &&colName : colNames)
    {
        if (colName == "genWeight")
        {
            has_genWeight = true;
        };
        if (colName == "LHEWeight_originalXWGTUP")
        {
            has_LHEWeight_originalXWGTUP = true;
        };
    }

    auto sum_pt = 0.;

    // df.Foreach(
    //     [&](ROOT::RVec<float> pt)
    //     {
    //         sum_pt += ROOT::VecOps::Sum(pt);
    //     },
    //     {"Muon_pt"});

    // save btag efficiency histograms
    // std::unique_ptr<TFile> btag_eff_maps_file(TFile::Open(
    //     fmt::format("btag_eff_maps_buffer/{}_{}.root", process_group, std::hash<std::string>{}(input_file)).c_str(),
    //     "RECREATE"));
    // btag_efficiency_light_num.Write();
    // btag_efficiency_light_den.Write();
    // btag_efficiency_c_num.Write();
    // btag_efficiency_c_den.Write();
    // btag_efficiency_b_num.Write();
    // btag_efficiency_b_den.Write();

    return sum_pt;
}
