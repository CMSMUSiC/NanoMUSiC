#ifndef VALIDATION_CONTAINER
#define VALIDATION_CONTAINER

#include "TTBarTo1Lep2Bjet2JetMET.hpp"
#include "WToLepNuX.hpp"
#include "ZToLepLepX.hpp"
#include "GammaPlusJet.hpp"
#include <string>

#define MERGE(analysis) analysis.merge_inplace(other->analysis)

class ValidationContainer
{

  public:
    ZToLepLepX z_to_muon_muon_x;
    ZToLepLepX z_to_muon_muon_x_z_mass;
    ZToLepLepX z_to_electron_electron_x;
    ZToLepLepX z_to_electron_electron_x_z_mass;
    ZToLepLepX z_to_tau_tau_x;
    ZToLepLepX z_to_tau_tau_x_z_mass;

    WToLepNuX w_to_muon_nutrino_x;
    WToLepNuX w_to_electron_nutrino_x;
    WToLepNuX w_to_tau_nutrino_x;

    TTBarTo1Lep2Bjet2JetMET ttbar_to_1muon_2bjet_2jet_met;
    TTBarTo1Lep2Bjet2JetMET ttbar_to_1electron_2bjet_2jet_met;
    TTBarTo1Lep2Bjet2JetMET ttbar_to_1tau_2bjet_2jet_met;

    GammaPlusJet gamma_plus_jets;

    ValidationContainer() = default;

    ValidationContainer(const std::string &process_group,
                        const std::string &xs_order,
                        const std::string &process,
                        const std::string &year)
    {
        z_to_muon_muon_x = ZToLepLepX(ZToLepLepX::Leptons::MUONS, false, process_group, xs_order, process, year);
        z_to_muon_muon_x_z_mass = ZToLepLepX(ZToLepLepX::Leptons::MUONS, true, process_group, xs_order, process, year);
        z_to_electron_electron_x =
            ZToLepLepX(ZToLepLepX::Leptons::ELECTRONS, false, process_group, xs_order, process, year);
        z_to_electron_electron_x_z_mass =
            ZToLepLepX(ZToLepLepX::Leptons::ELECTRONS, true, process_group, xs_order, process, year);
        z_to_tau_tau_x = ZToLepLepX(ZToLepLepX::Leptons::TAUS, false, process_group, xs_order, process, year);
        z_to_tau_tau_x_z_mass = ZToLepLepX(ZToLepLepX::Leptons::TAUS, true, process_group, xs_order, process, year);

        w_to_muon_nutrino_x = WToLepNuX(WToLepNuX::Leptons::MUONS, process_group, xs_order, process, year);
        w_to_electron_nutrino_x = WToLepNuX(WToLepNuX::Leptons::ELECTRONS, process_group, xs_order, process, year);
        w_to_tau_nutrino_x = WToLepNuX(WToLepNuX::Leptons::TAUS, process_group, xs_order, process, year);

        ttbar_to_1tau_2bjet_2jet_met =
            TTBarTo1Lep2Bjet2JetMET(TTBarTo1Lep2Bjet2JetMET::Leptons::MUONS, process_group, xs_order, process, year);
        ttbar_to_1tau_2bjet_2jet_met = TTBarTo1Lep2Bjet2JetMET(
            TTBarTo1Lep2Bjet2JetMET::Leptons::ELECTRONS, process_group, xs_order, process, year);
        ttbar_to_1tau_2bjet_2jet_met =
            TTBarTo1Lep2Bjet2JetMET(TTBarTo1Lep2Bjet2JetMET::Leptons::TAUS, process_group, xs_order, process, year);

        gamma_plus_jets =
            GammaPlusJet(process_group, xs_order, process, year);

    }

    auto serialize_to_root(const std::string output_filepath) -> std::vector<std::string>
    {
        auto analysis_names = std::vector<std::string>();

        std::unique_ptr<TFile> output_file(TFile::Open(output_filepath.c_str(), "RECREATE"));
        z_to_muon_muon_x.serialize_to_root(output_file);
        analysis_names.push_back(z_to_muon_muon_x.analysis_name);

        z_to_muon_muon_x_z_mass.serialize_to_root(output_file);
        analysis_names.push_back(z_to_muon_muon_x_z_mass.analysis_name);

        z_to_electron_electron_x.serialize_to_root(output_file);
        analysis_names.push_back(z_to_electron_electron_x.analysis_name);

        z_to_electron_electron_x_z_mass.serialize_to_root(output_file);
        analysis_names.push_back(z_to_electron_electron_x_z_mass.analysis_name);

        z_to_tau_tau_x.serialize_to_root(output_file);
        analysis_names.push_back(z_to_tau_tau_x.analysis_name);

        z_to_tau_tau_x_z_mass.serialize_to_root(output_file);
        analysis_names.push_back(z_to_tau_tau_x_z_mass.analysis_name);

        w_to_muon_nutrino_x.serialize_to_root(output_file);
        analysis_names.push_back(w_to_muon_nutrino_x.analysis_name);

        w_to_electron_nutrino_x.serialize_to_root(output_file);
        analysis_names.push_back(w_to_electron_nutrino_x.analysis_name);

        w_to_tau_nutrino_x.serialize_to_root(output_file);
        analysis_names.push_back(w_to_tau_nutrino_x.analysis_name);

        ttbar_to_1muon_2bjet_2jet_met.serialize_to_root(output_file);
        analysis_names.push_back(ttbar_to_1muon_2bjet_2jet_met.analysis_name);

        ttbar_to_1electron_2bjet_2jet_met.serialize_to_root(output_file);
        analysis_names.push_back(ttbar_to_1electron_2bjet_2jet_met.analysis_name);

        ttbar_to_1tau_2bjet_2jet_met.serialize_to_root(output_file);
        analysis_names.push_back(ttbar_to_1tau_2bjet_2jet_met.analysis_name);

        gamma_plus_jets.serialize_to_root(output_file);
        analysis_names.push_back(gamma_plus_jets.analysis_name);

        return analysis_names;
    }

    auto merge_inplace(const std::unique_ptr<ValidationContainer> &other) -> void
    {
        MERGE(z_to_muon_muon_x);
        MERGE(z_to_muon_muon_x_z_mass);
        MERGE(z_to_electron_electron_x);
        MERGE(z_to_electron_electron_x_z_mass);
        MERGE(z_to_tau_tau_x);
        MERGE(z_to_tau_tau_x_z_mass);
        MERGE(w_to_muon_nutrino_x);
        MERGE(w_to_electron_nutrino_x);
        MERGE(w_to_tau_nutrino_x);
        MERGE(ttbar_to_1muon_2bjet_2jet_met);
        MERGE(ttbar_to_1electron_2bjet_2jet_met);
        MERGE(ttbar_to_1tau_2bjet_2jet_met);
        MERGE(gamma_plus_jets);
    }
};

#endif // !VALIDATION_CONTAINER
