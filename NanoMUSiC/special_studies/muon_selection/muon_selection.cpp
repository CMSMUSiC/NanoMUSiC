#include "muon_selection.hpp"
#include "RunLumiFilter.hpp"

#include <cstdlib>

#include <chrono>
#include <mutex>
#include <thread>

auto muon_selection() -> int
{

    const int number_of_files = 10;
    const unsigned long max_events = 0;

    auto n_slots = 1;
    if (max_events == 0)
    {
        n_slots = 120;
    }

    std::vector<TH1F> h_pt;
    std::vector<TH1F> h_pt_low;
    std::vector<TH1F> h_pt_high;
    std::vector<TH1F> h_mass;
    std::vector<TH2F> h_mass_pt;
    for (std::size_t i = 0; i < static_cast<std::size_t>(n_slots); i++)
    {
        h_pt.push_back(TH1F(fmt::format("h_pt_{}", i).c_str(), //
                            fmt::format("h_pt_{}", i).c_str(), //
                            100,                               //
                            0.,                                //
                            2000.                              //
                            ));
        h_pt.at(i).Sumw2();

        h_pt_low.push_back(TH1F(fmt::format("h_pt_low_{}", i).c_str(), //
                                fmt::format("h_pt_low_{}", i).c_str(), //
                                100,                                   //
                                0.,                                    //
                                2000.                                  //
                                ));
        h_pt_low.at(i).Sumw2();

        h_pt_high.push_back(TH1F(fmt::format("h_pt_high_{}", i).c_str(), //
                                 fmt::format("h_pt_high_{}", i).c_str(), //
                                 100,                                    //
                                 0.,                                     //
                                 2000.                                   //
                                 ));
        h_pt_high.at(i).Sumw2();

        h_mass.push_back(TH1F(fmt::format("h_mass_{}", i).c_str(), //
                              fmt::format("h_mass_{}", i).c_str(), //
                              100,                                 //
                              0.,                                  //
                              1000.                                //
                              ));
        h_mass.at(i).Sumw2();

        h_mass_pt.push_back(TH2F(fmt::format("h_mass_pt_{}", i).c_str(), //
                                 fmt::format("h_mass_pt_{}", i).c_str(), //
                                 100,                                    //
                                 0.,                                     //
                                 1000.,
                                 100,  //
                                 0.,   //
                                 2000. //                                 //
                                 ));
        h_mass_pt.at(i).Sumw2();
    }

    // mutex for event_counter
    std::mutex mtx;
    bool loop_is_done = false;
    auto event_counters = std::vector<unsigned long>(n_slots, 0);
    fmt::print("Launching monitoring thread ...\n");
    std::thread monitoring_thread(event_counter, std::ref(mtx), std::ref(loop_is_done), std::ref(event_counters));

    auto run_lumi_filter = RunLumiFilter(
        "/.automount/home/home__home1/institut_3a/silva/projects/music/NanoMUSiC/configs/golden_jsons/Run2018.txt");

    auto loop = [&h_pt, &h_pt_low, &h_pt_high, &h_mass, &h_mass_pt, &event_counters, &run_lumi_filter]( //
                    unsigned int slot,                                                                  //
                    unsigned int luminosityBlock,                                                       //
                    unsigned int run,                                                                   //
                    int PV_npvsGood,                                                                    //
                    bool Flag_goodVertices,                                                             //
                    bool Flag_globalSuperTightHalo2016Filter,                                           //
                    bool Flag_HBHENoiseFilter,                                                          //
                    bool Flag_HBHENoiseIsoFilter,                                                       //
                    bool Flag_EcalDeadCellTriggerPrimitiveFilter,                                       //
                    bool Flag_BadPFMuonFilter,                                                          //
                    bool Flag_BadPFMuonDzFilter,                                                        //
                    bool Flag_eeBadScFilter,                                                            //
                    bool Flag_ecalBadCalibFilter,                                                       //
                    RVec<float> &Muon_pt,                                                               //
                    RVec<float> &Muon_eta,                                                              //
                    RVec<float> &Muon_phi,                                                              //
                    RVec<bool> &Muon_tightId,                                                           //
                    RVec<UChar_t> &Muon_highPtId,                                                       //
                    RVec<float> &Muon_pfRelIso04_all,                                                   //
                    RVec<float> &Muon_tkRelIso,                                                         //
                    RVec<float> &Muon_tunepRelPt,                                                       //
                    bool HLT_IsoMu24,                                                                   //
                    bool HLT_Mu50,                                                                      //
                    bool HLT_TkMu100,                                                                   //
                    bool HLT_OldMu100                                                                   //
                    ) -> void
    {
        bool is_good_run_lumi = run_lumi_filter(run, luminosityBlock, true);
        bool is_good_npv = PV_npvsGood;
        bool is_good_met_filters = Flag_goodVertices && Flag_globalSuperTightHalo2016Filter && Flag_HBHENoiseFilter &&
                                   Flag_HBHENoiseIsoFilter && Flag_EcalDeadCellTriggerPrimitiveFilter &&
                                   Flag_BadPFMuonFilter && Flag_BadPFMuonDzFilter && Flag_eeBadScFilter &&
                                   Flag_ecalBadCalibFilter;

        if (Muon_pt.size() > 0 and is_good_run_lumi and is_good_npv and is_good_met_filters)
        {
            bool pass_low_pt_trigger = HLT_IsoMu24;
            bool pass_high_pt_trigger = HLT_Mu50 || HLT_TkMu100 || HLT_OldMu100;

            auto leading_muon = Math::PtEtaPhiMVector(Muon_pt[0],  //
                                                      Muon_eta[0], //
                                                      Muon_phi[0], //
                                                      muon_mass);

            bool is_good_low_pt_muon = (Muon_pt[0] >= 25.)                //
                                       && (Muon_pt[0] < 200.)             //
                                       && (std::fabs(Muon_eta[0]) <= 2.4) //
                                       && (Muon_tightId[0])               //
                                       && (Muon_pfRelIso04_all[0] < 0.15);

            bool is_good_high_pt_muon = (Muon_pt[0] >= 200.)               //
                                        && (std::fabs(Muon_eta[0]) <= 2.4) //
                                        && (Muon_highPtId[0] >= 2)         //
                                        && (Muon_tkRelIso[0] < 0.10);

            if (is_good_high_pt_muon)
            {
                // for some reason, the the Relative pT Tune can yield very low corrected pT. Because of this,
                // they will be caped to 25., in order to not break the JSON SFs bound checks.
                // https://cms-nanoaod-integration.web.cern.ch/commonJSONSFs/summaries/MUO_2016postVFP_UL_muon_Z.html
                leading_muon.SetPt(std::max(Muon_tunepRelPt[0] * leading_muon.pt(), 25.));
            }

            if ((pass_low_pt_trigger and is_good_low_pt_muon) or (pass_high_pt_trigger and is_good_high_pt_muon))
            {
                h_pt[slot].Fill(leading_muon.pt(), 1.);
            }

            if (pass_low_pt_trigger and is_good_low_pt_muon)
            {
                h_pt_low[slot].Fill(leading_muon.pt(), 1.);
            }

            if (pass_high_pt_trigger and is_good_high_pt_muon)
            {
                h_pt_high[slot].Fill(leading_muon.pt(), 1.);
            }

            // dimuon check
            if (Muon_pt.size() > 1)
            {
                auto sub_leading_muon = Math::PtEtaPhiMVector(Muon_pt[1],  //
                                                              Muon_eta[1], //
                                                              Muon_phi[1], //
                                                              muon_mass);
                is_good_low_pt_muon = (Muon_pt[1] >= 25.)                //
                                      && (Muon_pt[1] < 200.)             //
                                      && (std::fabs(Muon_eta[1]) <= 2.4) //
                                      && (Muon_tightId[1])               //
                                      && (Muon_pfRelIso04_all[1] < 0.15);

                is_good_high_pt_muon = (Muon_pt[1] >= 200.)               //
                                       && (std::fabs(Muon_eta[1]) <= 2.4) //
                                       && (Muon_highPtId[1] >= 2)         //
                                       && (Muon_tkRelIso[1] < 0.10);

                if (is_good_high_pt_muon)
                {
                    // for some reason, the the Relative pT Tune can yield very low corrected pT. Because of this,
                    // they will be caped to 25., in order to not break the JSON SFs bound checks.
                    // https://cms-nanoaod-integration.web.cern.ch/commonJSONSFs/summaries/MUO_2016postVFP_UL_muon_Z.html
                    sub_leading_muon.SetPt(std::max(Muon_tunepRelPt[1] * sub_leading_muon.pt(), 25.));
                }

                if ((pass_low_pt_trigger and is_good_low_pt_muon) or (pass_high_pt_trigger and is_good_high_pt_muon))
                {
                    h_mass[slot].Fill((leading_muon + sub_leading_muon).mass(), 1.);
                    h_mass_pt[slot].Fill((leading_muon + sub_leading_muon).mass(), leading_muon.pt(), 1.);
                }
            }
        }

        event_counters[slot]++;
    };

    if (max_events > 0)
    {
        RDataFrame df("Events", input_files(number_of_files));

        fmt::print("Starting event loop [Serial] ...\n");
        df.Range(max_events).ForeachSlot(loop, cols());
        loop_is_done = true;
    }
    else
    {
        // Tell ROOT to go parallel
        ROOT::EnableImplicitMT(n_slots);

        RDataFrame df("Events", input_files(number_of_files));

        fmt::print("Starting event loop [Parallel] ...\n");
        df.ForeachSlot(loop, cols());
        loop_is_done = true;
    }
    fmt::print("Joinning monitoring thread ...\n");
    monitoring_thread.join();

    save_as(h_pt, "histo_pt");
    save_as(h_pt_low, "histo_pt_low");
    save_as(h_pt_high, "histo_pt_high");
    save_as(h_mass, "h_mass");
    save_as(h_mass_pt, "h_mass_pt");
    fmt::print("... done.\n");

    return EXIT_SUCCESS;
}
