#include "ROOT/RVec.hxx"
#include "TTree.h"
#include "fmt/format.h"

using namespace ROOT::VecOps;

void lhe_particles_inspector()
{
    // TFile *_file0 = TFile::Open(
    //     "root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL16NanoAODAPVv9/"
    //     "WGJets_MonoPhoton_PtG-40to130_TuneCP5_13TeV-madgraph-pythia8/NANOAODSIM/106X_mcRun2_asymptotic_preVFP_v11-v2/"
    //     "40000/E54F6BA5-D0B0-4844-9475-94F4AB1ABE72.root");

    TFile *_file0 = TFile::Open(
        "root://cms-xrd-global.cern.ch///store/mc/RunIISummer20UL18NanoAODv9/"
        "WGToLNuG_01J_5f_PtG_130_TuneCP5_13TeV-amcatnloFXFX-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v16_L1v1-v1/"
        "130000/69972AEE-D53B-FF40-9925-3424B94F337A.root");

    auto t = _file0->Get<TTree>("Events");

    auto df = ROOT::RDataFrame(*t, {"LHEPart_pdgId", "LHEPart_status", "LHEPart_pt"});
    ULong64_t total = 0;
    ULong64_t accepted = 0;

    fmt::print("Starting ...\n");
    df.Foreach(
        [&](ROOT::RVec<int> pdgId, ROOT::RVec<int> status, ROOT::RVec<float> pt) {
            total++;
            // std::cout << "------------------------" << std::endl;
            // std::cout << pdg_id << std::endl;
            // std::cout << status << std::endl;
            // std::cout << pt << std::endl;
            auto filter = [&](const float &pt_max) -> bool {
                if (ROOT::VecOps::Any(pt[(pdgId == 22) && (status == 1)] <= pt_max))
                {
                    // fmt::print("[ Generator Filter ] Pass: TTBar Mass Cut. ...\n");
                    return true;
                }

                // fmt::print("[ Generator Filter ] Didn't pass: TTBar Mass Cut. Skipping ...\n");
                return false;
            };

            if (filter(150.))
            {
                accepted++;
            }
        },
        {"LHEPart_pdgId", "LHEPart_status", "LHEPart_pt"});

    fmt::print("Filter result: {}\n", static_cast<float>(accepted) / static_cast<float>(total));

    std::cout << _file0->GetName() << std::endl;
}