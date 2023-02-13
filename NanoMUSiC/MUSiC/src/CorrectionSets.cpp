#include "CorrectionSets.hpp"

auto ElectronTriggerSF::read_xxd_dump(const unsigned char arr[], const unsigned int _size, const std::string &name)
    -> TMemFile
{
    std::unique_ptr<char[]> buffer(new char[_size]());

    for (unsigned int i = 0; i < _size; i++)
    {
        buffer[i] = static_cast<char>(arr[i]);
    }

    return {name.c_str(), buffer.get(), _size};
}

//////////////////////////////////////////
/// Get limits for a given 2D SF histogram
///
auto ElectronTriggerSF::get_limits(const TH2F &histo) -> std::tuple<double, double, double, double>
{

    auto eta_upper_limit = histo.GetXaxis()->GetBinUpEdge(histo.GetXaxis()->GetLast());
    auto eta_lower_limit = sf_histogram_barrel.GetXaxis()->GetBinLowEdge(sf_histogram_barrel.GetXaxis()->GetFirst());

    auto pt_upper_limit = histo.GetYaxis()->GetBinUpEdge(histo.GetYaxis()->GetLast());
    auto pt_lower_limit = histo.GetYaxis()->GetBinLowEdge(histo.GetYaxis()->GetFirst());

    return std::make_tuple(eta_upper_limit, //
                           eta_lower_limit, //
                           pt_upper_limit,  //
                           pt_lower_limit);
}
// Reference:
// https://twiki.cern.ch/twiki/bin/view/CMS/EgHLTScaleFactorMeasurements
ElectronTriggerSF::ElectronTriggerSF(const PtRegime &_pt_regime, const Year &_year)
    : pt_regime(_pt_regime),
      year(_year)
{
    // histo->GetXaxis()->GetBinUpEdge(histo->GetXaxis()->GetLast())
    if (year == Year::Run2016APV)
    {
        if (pt_regime == PtRegime::LowPt)
        {
            auto memory_file = read_xxd_dump(EGTriggerLowPtRun2016APV::egammaEffi_txt_EGM2D_root,
                                             EGTriggerLowPtRun2016APV::egammaEffi_txt_EGM2D_root_len);
            // get histograms
            sf_histogram_barrel = *(memory_file.Get<TH2F>("EGamma_SF2D"));
            sf_histogram_endcap = *(memory_file.Get<TH2F>("EGamma_SF2D"));

            // get limits
            auto barrel_limits = get_limits(sf_histogram_barrel);
            auto endcap_limits = get_limits(sf_histogram_endcap);
            // eta
            barrel_eta_upper_limit = std::get<0>(barrel_limits);
            barrel_eta_lower_limit = std::get<1>(barrel_limits);
            barrel_pt_upper_limit = std::get<2>(barrel_limits);
            barrel_pt_lower_limit = std::get<3>(barrel_limits);
            // pt
            endcap_eta_upper_limit = std::get<0>(endcap_limits);
            endcap_eta_lower_limit = std::get<1>(endcap_limits);
            endcap_pt_upper_limit = std::get<2>(endcap_limits);
            endcap_pt_lower_limit = std::get<3>(endcap_limits);
        }
        else
        {
            auto memory_file = read_xxd_dump(EGTriggerHighPtRun2016APV::egammaEffi_txt_EGM2D_root,
                                             EGTriggerHighPtRun2016APV::egammaEffi_txt_EGM2D_root_len);
            sf_histogram_barrel = *(memory_file.Get<TH2F>("SF_TH2F_Barrel"));
            sf_histogram_endcap = *(memory_file.Get<TH2F>("SF_TH2F_EndCap"));

            // get limits
            auto barrel_limits = get_limits(sf_histogram_barrel);
            auto endcap_limits = get_limits(sf_histogram_endcap);
            // eta
            barrel_eta_upper_limit = std::get<0>(barrel_limits);
            barrel_eta_lower_limit = std::get<1>(barrel_limits);
            barrel_pt_upper_limit = std::get<2>(barrel_limits);
            barrel_pt_lower_limit = std::get<3>(barrel_limits);
            // pt
            endcap_eta_upper_limit = std::get<0>(endcap_limits);
            endcap_eta_lower_limit = std::get<1>(endcap_limits);
            endcap_pt_upper_limit = std::get<2>(endcap_limits);
            endcap_pt_lower_limit = std::get<3>(endcap_limits);
        }
    }
    else if (year == Year::Run2016)
    {
        if (pt_regime == PtRegime::LowPt)
        {
            auto memory_file = read_xxd_dump(EGTriggerLowPtRun2016::egammaEffi_txt_EGM2D_root,
                                             EGTriggerLowPtRun2016::egammaEffi_txt_EGM2D_root_len);
            sf_histogram_barrel = *(memory_file.Get<TH2F>("EGamma_SF2D"));
            sf_histogram_endcap = *(memory_file.Get<TH2F>("EGamma_SF2D"));

            // get limits
            auto barrel_limits = get_limits(sf_histogram_barrel);
            auto endcap_limits = get_limits(sf_histogram_endcap);
            // eta
            barrel_eta_upper_limit = std::get<0>(barrel_limits);
            barrel_eta_lower_limit = std::get<1>(barrel_limits);
            barrel_pt_upper_limit = std::get<2>(barrel_limits);
            barrel_pt_lower_limit = std::get<3>(barrel_limits);
            // pt
            endcap_eta_upper_limit = std::get<0>(endcap_limits);
            endcap_eta_lower_limit = std::get<1>(endcap_limits);
            endcap_pt_upper_limit = std::get<2>(endcap_limits);
            endcap_pt_lower_limit = std::get<3>(endcap_limits);
        }
        else
        {
            auto memory_file = read_xxd_dump(EGTriggerHighPtRun2016::egammaEffi_txt_EGM2D_root,
                                             EGTriggerHighPtRun2016::egammaEffi_txt_EGM2D_root_len);
            sf_histogram_barrel = *(memory_file.Get<TH2F>("SF_TH2F_Barrel"));
            sf_histogram_endcap = *(memory_file.Get<TH2F>("SF_TH2F_EndCap"));

            // get limits
            auto barrel_limits = get_limits(sf_histogram_barrel);
            auto endcap_limits = get_limits(sf_histogram_endcap);
            // eta
            barrel_eta_upper_limit = std::get<0>(barrel_limits);
            barrel_eta_lower_limit = std::get<1>(barrel_limits);
            barrel_pt_upper_limit = std::get<2>(barrel_limits);
            barrel_pt_lower_limit = std::get<3>(barrel_limits);
            // pt
            endcap_eta_upper_limit = std::get<0>(endcap_limits);
            endcap_eta_lower_limit = std::get<1>(endcap_limits);
            endcap_pt_upper_limit = std::get<2>(endcap_limits);
            endcap_pt_lower_limit = std::get<3>(endcap_limits);
        }
    }
    else if (year == Year::Run2017)
    {
        if (pt_regime == PtRegime::LowPt)
        {
            auto memory_file = read_xxd_dump(EGTriggerLowPtRun2017::egammaEffi_txt_EGM2D_root,
                                             EGTriggerLowPtRun2017::egammaEffi_txt_EGM2D_root_len);
            sf_histogram_barrel = *(memory_file.Get<TH2F>("EGamma_SF2D"));
            sf_histogram_endcap = *(memory_file.Get<TH2F>("EGamma_SF2D"));

            // get limits
            auto barrel_limits = get_limits(sf_histogram_barrel);
            auto endcap_limits = get_limits(sf_histogram_endcap);
            // eta
            barrel_eta_upper_limit = std::get<0>(barrel_limits);
            barrel_eta_lower_limit = std::get<1>(barrel_limits);
            barrel_pt_upper_limit = std::get<2>(barrel_limits);
            barrel_pt_lower_limit = std::get<3>(barrel_limits);
            // pt
            endcap_eta_upper_limit = std::get<0>(endcap_limits);
            endcap_eta_lower_limit = std::get<1>(endcap_limits);
            endcap_pt_upper_limit = std::get<2>(endcap_limits);
            endcap_pt_lower_limit = std::get<3>(endcap_limits);
        }
        else
        {
            auto memory_file = read_xxd_dump(EGTriggerHighPtRun2017::egammaEffi_txt_EGM2D_root,
                                             EGTriggerHighPtRun2017::egammaEffi_txt_EGM2D_root_len);
            sf_histogram_barrel = *(memory_file.Get<TH2F>("SF_TH2F_Barrel"));
            sf_histogram_endcap = *(memory_file.Get<TH2F>("SF_TH2F_EndCap"));

            // get limits
            auto barrel_limits = get_limits(sf_histogram_barrel);
            auto endcap_limits = get_limits(sf_histogram_endcap);
            // eta
            barrel_eta_upper_limit = std::get<0>(barrel_limits);
            barrel_eta_lower_limit = std::get<1>(barrel_limits);
            barrel_pt_upper_limit = std::get<2>(barrel_limits);
            barrel_pt_lower_limit = std::get<3>(barrel_limits);
            // pt
            endcap_eta_upper_limit = std::get<0>(endcap_limits);
            endcap_eta_lower_limit = std::get<1>(endcap_limits);
            endcap_pt_upper_limit = std::get<2>(endcap_limits);
            endcap_pt_lower_limit = std::get<3>(endcap_limits);
        }
    }
    else if (year == Year::Run2018)
    {
        if (pt_regime == PtRegime::LowPt)
        {
            auto memory_file = read_xxd_dump(EGTriggerLowPtRun2018::egammaEffi_txt_EGM2D_root,
                                             EGTriggerLowPtRun2018::egammaEffi_txt_EGM2D_root_len);
            sf_histogram_barrel = *(memory_file.Get<TH2F>("EGamma_SF2D"));
            sf_histogram_endcap = *(memory_file.Get<TH2F>("EGamma_SF2D"));

            // get limits
            auto barrel_limits = get_limits(sf_histogram_barrel);
            auto endcap_limits = get_limits(sf_histogram_endcap);
            // eta
            barrel_eta_upper_limit = std::get<0>(barrel_limits);
            barrel_eta_lower_limit = std::get<1>(barrel_limits);
            barrel_pt_upper_limit = std::get<2>(barrel_limits);
            barrel_pt_lower_limit = std::get<3>(barrel_limits);
            // pt
            endcap_eta_upper_limit = std::get<0>(endcap_limits);
            endcap_eta_lower_limit = std::get<1>(endcap_limits);
            endcap_pt_upper_limit = std::get<2>(endcap_limits);
            endcap_pt_lower_limit = std::get<3>(endcap_limits);
        }
        else
        {
            auto memory_file = read_xxd_dump(EGTriggerHighPtRun2018::egammaEffi_txt_EGM2D_root,
                                             EGTriggerHighPtRun2018::egammaEffi_txt_EGM2D_root_len);
            sf_histogram_barrel = *(memory_file.Get<TH2F>("SF_TH2F_Barrel"));
            sf_histogram_endcap = *(memory_file.Get<TH2F>("SF_TH2F_EndCap"));

            // get limits
            auto barrel_limits = get_limits(sf_histogram_barrel);
            auto endcap_limits = get_limits(sf_histogram_endcap);
            // eta
            barrel_eta_upper_limit = std::get<0>(barrel_limits);
            barrel_eta_lower_limit = std::get<1>(barrel_limits);
            barrel_pt_upper_limit = std::get<2>(barrel_limits);
            barrel_pt_lower_limit = std::get<3>(barrel_limits);
            // pt
            endcap_eta_upper_limit = std::get<0>(endcap_limits);
            endcap_eta_lower_limit = std::get<1>(endcap_limits);
            endcap_pt_upper_limit = std::get<2>(endcap_limits);
            endcap_pt_lower_limit = std::get<3>(endcap_limits);
        }
    }
    else
    {
        throw std::runtime_error("[ Electron Trigger SF ] A proper year and pt_regime combination was not provided.");
    }

    // print configurations
    // PtRegime pt_regime;
    // Year year;
    // TH2F sf_histogram_barrel;
    // TH2F sf_histogram_endcap;
    // float barrel_eta_upper_limit;
    // float barrel_eta_lower_limit;
    // float endcap_eta_upper_limit;
    // float endcap_eta_lower_limit;
    // float barrel_pt_upper_limit;
    // float barrel_pt_lower_limit;
    // float endcap_pt_upper_limit;
    // float endcap_pt_lower_limit;
    fmt::print("\n=========================================================\n");
    fmt::print("------------      Electron Trigger SF    ----------------\n");
    fmt::print("Energy Regime: {}\n", (_pt_regime == PtRegime::LowPt) ? "Low pT"sv : "High pT"sv);
    fmt::print("Barrel eta: [{} - {}]\n", barrel_eta_lower_limit, barrel_eta_upper_limit);
    fmt::print("Barrel pT: [{} - {}]\n", barrel_pt_lower_limit, barrel_pt_upper_limit);
    fmt::print("Endcap eta: [{} - {}]\n", endcap_eta_lower_limit, endcap_eta_upper_limit);
    fmt::print("Endcap pT: [{} - {}]\n", endcap_pt_lower_limit, endcap_pt_upper_limit);
    fmt::print("=========================================================\n\n");
}

auto ElectronTriggerSF::operator()(float eta_sc, float pt, std::string_view variation) const -> double
{
    // check for physical values
    if (pt < 0.)
    {
        throw std::runtime_error("[ Electron Trigger SF ] The value of pT should be positive.");
    }

    // check if is barrel or endcap
    if (std::fabs(eta_sc) < 1.442)
    {
        // check limits
        if (pt >= barrel_pt_upper_limit)
        {
            pt = barrel_pt_upper_limit - epsilon;
        }
        if (pt <= barrel_pt_lower_limit)
        {
            pt = barrel_pt_lower_limit + epsilon;
        }
        auto nominal = sf_histogram_barrel.GetBinContent(sf_histogram_barrel.FindFixBin(eta_sc, pt));
        auto uncert = 0.;

        if (variation == "up")
        {
            uncert = sf_histogram_barrel.GetBinErrorUp(sf_histogram_barrel.FindFixBin(eta_sc, pt));
        }
        if (variation == "down")
        {
            uncert = -1.0 * sf_histogram_barrel.GetBinErrorLow(sf_histogram_barrel.FindFixBin(eta_sc, pt));
        }

        return nominal + uncert;
    }
    if (std::fabs(eta_sc) >= 1.566 and std::fabs(eta_sc) < 2.5)
    {
        // check limits
        if (pt >= endcap_pt_upper_limit)
        {
            pt = endcap_pt_upper_limit - epsilon;
        }
        if (pt <= endcap_pt_lower_limit)
        {
            pt = endcap_pt_lower_limit + epsilon;
        }

        auto nominal = sf_histogram_endcap.GetBinContent(sf_histogram_endcap.FindFixBin(eta_sc, pt));
        auto uncert = 0.;

        if (variation == "up")
        {
            uncert = sf_histogram_barrel.GetBinErrorUp(sf_histogram_barrel.FindFixBin(eta_sc, pt));
        }
        if (variation == "down")
        {
            uncert = -1.0 * sf_histogram_barrel.GetBinErrorLow(sf_histogram_barrel.FindFixBin(eta_sc, pt));
        }

        return nominal + uncert;
    }
    throw std::runtime_error(
        fmt::format("[ Electron Trigger SF ] The provided Super Cluster eta value ({}), is about of range.", eta_sc));
}

namespace CorrectionHelpers
{

///////////////////////////////////////////////////////////////
/// For some reason, the Official Muon SFs requires a field of the requested year, with proper formating.
auto get_year_for_muon_sf(Year year) -> std::string
{
    switch (year)
    {
    case Year::Run2016APV:
        return "2016preVFP_UL"s;
    case Year::Run2016:
        return "2016postVFP_UL"s;
    case Year::Run2017:
        return "2017_UL"s;
    case Year::Run2018:
        return "2018_UL"s;
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

} // namespace CorrectionHelpers

using CorrectionlibRef_t = correction::Correction::Ref;
using RochesterCorrection_t = RoccoR;
using ElectronTriggerSF_t = ElectronTriggerSF;

Corrector::Corrector(const std::string_view &_correction_type, const Year _year, bool _is_data)
    : correction_type(_correction_type),
      year(_year),
      is_data(_is_data)
{
    // Rochester Correction
    if (_correction_type == "MuonLowPt")
    {
        auto [input_file, _dummy_key] = correction_keys.at({correction_type, year});
        correction_ref = RoccoR(input_file);
    }

    // Single Electron Trigger
    else if (_correction_type == "SingleElectronLowPt")
    {
        correction_ref = ElectronTriggerSF(ElectronTriggerSF::PtRegime::LowPt, year);
    }
    else if (_correction_type == "SingleElectronHighPt")
    {
        correction_ref = ElectronTriggerSF(ElectronTriggerSF::PtRegime::HighPt, year);
    }

    // default case is any correction that uses the correctionlib
    else
    {
        auto [json_file, key] = correction_keys.at({correction_type, year});
        correction_ref = correction::CorrectionSet::from_file(json_file)->at(key);
    }
}

auto Corrector::operator()() const -> double
{
    if (not is_data)
    {
        return 1.;
    }
    return 1.;
}

// Electron Trigger SF
auto Corrector::operator()(const float &eta_sc, const float &pt, const std::string_view &variation) const -> double
{
    if (not is_data)
    {
        return std::get<ElectronTriggerSF_t>(correction_ref)(eta_sc, pt, variation);
    }
    return 1.;
}

// rochester corrections - Data
auto Corrector::operator()(const int Q, const double pt, const double eta, const double phi, const int s, const int m)
    const -> double
{
    if (is_data)
    {
        return std::get<RochesterCorrection_t>(correction_ref).kScaleDT(Q, pt, eta, phi, s, m);
    }
    throw std::runtime_error("This signature is only valid for Data samples.");
}

// rochester corrections - MC (matched GenMuon - recommended)
auto Corrector::operator()(const int Q,
                           const double pt,
                           const double eta,
                           const double phi,
                           const double genPt,
                           const int s,
                           const int m) const -> double
{
    if (not is_data)
    {
        return std::get<RochesterCorrection_t>(correction_ref).kSpreadMC(Q, pt, eta, phi, genPt, s, m);
    }
    throw std::runtime_error("This signature is only valid for MC samples.");
}

// rochester corrections - MC (unmatched GenMuon - not recommended)
auto Corrector::operator()(const int Q,
                           const double pt,
                           const double eta,
                           const double phi,
                           const int n,
                           const double u,
                           const int s,
                           const int m) const -> double
{
    if (not is_data)
    {
        return std::get<RochesterCorrection_t>(correction_ref).kSmearMC(Q, pt, eta, phi, n, u, s, m);
    }
    throw std::runtime_error("The signature of the used method is only valid for MC samples.");
}

ElectronSFCorrector::ElectronSFCorrector(const Year _year, bool _is_data)
    : year(_year),
      is_data(_is_data)
{
    switch (year)
    {
    case Year::Run2016APV:
        correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016preVFP_UL/electron.json.gz")
                ->at("UL-Electron-ID-SF");
        year_str = "2016preVFP"s;
        break;
    case Year::Run2016:
        correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016postVFP_UL/electron.json.gz")
                ->at("UL-Electron-ID-SF");
        year_str = "2016postVFP"s;
        break;
    case Year::Run2017:
        correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2017_UL/electron.json.gz")
                ->at("UL-Electron-ID-SF");
        year_str = "2017"s;
        break;
    case Year::Run2018:
        correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2018_UL/electron.json.gz")
                ->at("UL-Electron-ID-SF");
        year_str = "2018"s;
        break;
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

//////////////////////////////////
/// Get HEEP Id SF
/// Ref:
/// https://twiki.cern.ch/twiki/bin/view/CMS/HEEPElectronIdentificationRun2#Scale_Factor
auto ElectronSFCorrector::get_high_pt_sf(const Year &year,
                                         const std::string &variation,
                                         const float &pt,
                                         const float &eta) const -> float
{
    bool is_EB = false;
    if (std::fabs(eta) < 1.566)
    {
        is_EB = true;
    }
    else if (std::fabs(eta) <= 2.5)
    {
        is_EB = false;
    }
    else
    {
        throw std::runtime_error(fmt::format("The Eta SC ({}) value is out of range.", eta));
    }

    auto syst = [&](float a, float b, float c) -> float {
        if (pt < 90.)
            return a / 100.;
        else
            return 0.01 * std::min(1 + (pt - 90.f) * b, c);
    };

    auto syst_multiplier = [&]() -> float {
        if (variation == "sf")
        {
            return 0.;
        }
        if (variation == "sfup")
        {
            return 1.;
        }
        if (variation == "sfdown")
        {
            return -1.;
        }
        throw std::runtime_error(fmt::format("Invalid variation parameter ({}).", variation));
    };

    switch (year)
    {
        /// 2016 legacy: 0.983±0.000 (stat) (EB), 0.991±0.001 (stat) (EE) (taken from slide 10 of [0])
        ///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
        ///              uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)
    case Year::Run2016APV:
        if (is_EB)
        {
            return 0.983 + syst_multiplier() * std::sqrt(std::pow(0., 2) + std::pow(syst(1., 0.0022, 3.), 2));
        }
        return 0.991 + syst_multiplier() * std::sqrt(std::pow(0.001, 2) + std::pow(syst(2., 0.0143, 5.), 2));

    case Year::Run2016:
        if (is_EB)
        {
            return 0.983 + syst_multiplier() * std::sqrt(std::pow(0., 2) + std::pow(syst(1., 0.0022, 3.), 2));
        }
        return 0.991 + syst_multiplier() * std::sqrt(std::pow(0.001, 2) + std::pow(syst(2., 0.0143, 5.), 2));

        /// 2017 prompt: 0.968±0.001 (stat) (EB), 0.973±0.002 (stat) (EE)
        ///              uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
        ///              uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)
    case Year::Run2017:
        if (is_EB)
        {
            return 0.968 + syst_multiplier() * std::sqrt(std::pow(0.001, 2) + std::pow(syst(1., 0.0022, 3.), 2));
        }
        return 0.973 + syst_multiplier() * std::sqrt(std::pow(0.002, 2) + std::pow(syst(2., 0.0143, 5.), 2));

        /// 2018 rereco (Autumn 18): 0.969 +/- 0.000 (stat) (EB), and 0.984 +/- 0.001 (stat) (EE).
        ///                          uncertainty (syst?): EB ET < 90 GeV: 1% else min(1+(ET-90)*0.0022)%,3%)
        ///                          uncertainty (syst?): EE ET < 90 GeV: 2% else min(1+(ET-90)*0.0143)%,5%)
    case Year::Run2018:
        if (is_EB)
        {
            return 0.969 + syst_multiplier() * std::sqrt(std::pow(0., 2) + std::pow(syst(1., 0.0022, 3.), 2));
        }
        return 0.984 + syst_multiplier() * std::sqrt(std::pow(0.001, 2) + std::pow(syst(2., 0.0143, 5.), 2));

    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

PhotonSFCorrector::PhotonSFCorrector(const std::string &_correction_type, const Year _year, bool _is_data)
    : correction_type(_correction_type),
      year(_year),
      is_data(_is_data)
{

    std::string key = ""s;
    if (correction_type == "PhotonID"s)
    {
        key = "UL-Photon-ID-SF";
    }
    else if (correction_type == "PixelSeed"s)
    {
        key = "UL-Photon-PixVeto-SF";
    }
    else
    {
        throw std::runtime_error(fmt::format("Non valid key ({}) was provided.", correction_type));
    }

    switch (year)
    {
    case Year::Run2016APV:
        correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016preVFP_UL/photon.json.gz")
                ->at(key);
        year_str = "2016preVFP"s;
        break;
    case Year::Run2016:
        correction_ref =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2016postVFP_UL/photon.json.gz")
                ->at(key);
        year_str = "2016postVFP"s;
        break;
    case Year::Run2017:
        correction_ref = correction::CorrectionSet::from_file(
                             "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2017_UL/photon.json.gz")
                             ->at(key);
        year_str = "2017"s;
        break;
    case Year::Run2018:
        correction_ref = correction::CorrectionSet::from_file(
                             "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM/2018_UL/photon.json.gz")
                             ->at(key);
        year_str = "2018"s;
        break;
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

BTagSFCorrector::BTagSFCorrector(const Year _year, bool _is_data)
    : year(_year),
      is_data(_is_data)
{
    switch (year)
    {
    case Year::Run2016APV:
        correction_ref_bjet =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2016preVFP_UL/btagging.json.gz")
                ->at("deepJet_comb");
        correction_ref_light_jet =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2016preVFP_UL/btagging.json.gz")
                ->at("deepJet_incl");
        break;
    case Year::Run2016:
        correction_ref_bjet =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2016postVFP_UL/btagging.json.gz")
                ->at("deepJet_comb");
        correction_ref_light_jet =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2016postVFP_UL/btagging.json.gz")
                ->at("deepJet_incl");
        break;
    case Year::Run2017:
        correction_ref_bjet =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2017_UL/btagging.json.gz")
                ->at("deepJet_comb");
        correction_ref_light_jet =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2017_UL/btagging.json.gz")
                ->at("deepJet_incl");
        break;
    case Year::Run2018:
        correction_ref_bjet =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2018_UL/btagging.json.gz")
                ->at("deepJet_comb");
        correction_ref_light_jet =
            correction::CorrectionSet::from_file(
                "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/BTV/2018_UL/btagging.json.gz")
                ->at("deepJet_incl");
        break;
    default:
        throw std::runtime_error("Year (" + std::to_string(year) +
                                 ") not matching with any possible Run2 cases (2016APV, 2016, 2017 or 2018).");
    }
}

//////////////////////////////////////
/// get btag efficiencies
/// TODO: Produce these maps for each sample
/// For now we are taking the mean value of what was used for the 2016 papper
auto BTagSFCorrector::get_eff(const float &_pt, const float &_eta, const int &_flavour) const -> double
{
    if (not is_data)
    {
        if (_flavour == 5)
        {
            return 0.41;
        }
        if (_flavour == 4)
        {
            return 0.024;
        }
        return 0.002;
    }
    return 1.;
}

////////////////////////////////////////////////////////
/// Using Method 1A - Per event weight
/// References:
/// - https://twiki.cern.ch/twiki/bin/view/CMS/BTagSFMethods#1a_Event_reweighting_using_scale
/// - https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideCMSDataAnalysisSchoolLPC2023TaggingExercise
/// - https://github.com/IreneZoi/CMSDAS2023-BTV/tree/master/BTaggingExercise
/// - https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation#UltraLegacy_scale_factor_uncerta
///
/// systematic (string): central, down, down_correlated, down_uncorrelated, up, up_correlated,
/// working_point (string): L, M, T
/// flavor (int): 5=b, 4=c, 0=udsg
/// abseta (real)
/// pt (real)

auto BTagSFCorrector::operator()(const std::string &variation_bjet,          //
                                 const std::string &variation_light_jet,     //
                                 const RVec<float> &tagged_jets_pt,          //
                                 const RVec<float> &tagged_jets_abseta,      //
                                 const RVec<int> &tagged_jets_hadronFlavour, //
                                 const RVec<float> &untagged_jets_pt,        //
                                 const RVec<float> &untagged_jets_abseta,    //
                                 const RVec<int> &untagged_jets_hadronFlavour) const -> float
{

    if (not is_data)
    {
        RVec<float> pmc_tagged_vec = VecOps::Map(
            tagged_jets_pt,
            tagged_jets_abseta,
            tagged_jets_hadronFlavour,
            [&](const float &_pt, const float &_eta, const int &_flavour) { return get_eff(_pt, _eta, _flavour); });

        RVec<float> pmc_untagged_vec = VecOps::Map(untagged_jets_pt,
                                                   untagged_jets_abseta,
                                                   untagged_jets_hadronFlavour,
                                                   [&](const float &_pt, const float &_eta, const int &_flavour) {
                                                       return (1. - get_eff(_pt, _eta, _flavour));
                                                   });

        RVec<float> pdata_tagged_vec = VecOps::Map(
            tagged_jets_pt,
            tagged_jets_abseta,
            tagged_jets_hadronFlavour,
            [&](const float &_pt, const float &_eta, const int &_flavour) {
                if (_flavour == 0)
                {
                    return get_sf_light_jet({variation_light_jet, "T", _flavour, _eta, _pt}) *
                           get_eff(_pt, _eta, _flavour);
                }
                return get_sf_bjet({variation_bjet, "T", _flavour, _eta, _pt}) * get_eff(_pt, _eta, _flavour);
            });

        RVec<float> pdata_untagged_vec = VecOps::Map(
            untagged_jets_pt,
            untagged_jets_abseta,
            untagged_jets_hadronFlavour,
            [&](const float &_pt, const float &_eta, const int &_flavour) {
                if (_flavour == 0)
                {
                    return (1. - (get_sf_light_jet({variation_light_jet, "T", _flavour, _eta, _pt}) *
                                  get_eff(_pt, _eta, _flavour)));
                }
                return (1. - (get_sf_bjet({variation_bjet, "T", _flavour, _eta, _pt}) * get_eff(_pt, _eta, _flavour)));
            });

        float pmc = std::reduce(pmc_tagged_vec.cbegin(), pmc_tagged_vec.cend(), 1.f, std::multiplies<float>()) *
                    std::reduce(pmc_untagged_vec.cbegin(), pmc_untagged_vec.cend(), 1.f, std::multiplies<float>());

        float pdata =
            std::reduce(pdata_tagged_vec.cbegin(), pdata_tagged_vec.cend(), 1.f, std::multiplies<float>()) *
            std::reduce(pdata_untagged_vec.cbegin(), pdata_untagged_vec.cend(), 1.f, std::multiplies<float>());

        float weight = 1.0;
        if (pmc != 0.0)
        {
            weight = pdata / pmc;
        }

        // if (tagged_jets_pt.size() > 0 or untagged_jets_pt.size() > 0)
        // {
        // fmt::print("Weight (BTag): (weight) {} = {}/{} (pdata)/(pmc)\n", weight, pdata, pmc);
        // }

        return weight;
    }
    return 1.;
}
