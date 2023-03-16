#include "NanoMUSiC.hpp"
#include <optional>

auto main(int argc, char *argv[]) -> int
{
    // silence LHAPDF
    LHAPDF::setVerbosity(0);

    TDirectory::AddDirectory(kFALSE); // Force ROOT to give directories in our hand - Yes, we can
    TH1::AddDirectory(kFALSE);        // Force ROOT to give histograms in our hand - Yes, we can

    // command line options
    argh::parser cmdl(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
    const bool show_help = cmdl[{"-h", "--help"}];
    bool batch_mode = cmdl[{"-b", "--batch"}];
    const std::string run_config_file = cmdl({"-c", "--run-config"}).str();

    if (show_help || run_config_file == "")
    {
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << "MUSiC - Model Unspecific Search in CMS" << std::endl;
        std::cout << emojicpp::emojize("      :signal_strength: Run2 - Ultra Legacy :signal_strength:") << std::endl;
        std::cout << " " << std::endl;

        std::cout << run_config_file << std::endl;
        if (run_config_file == "")
        {
            std::cout << "ERROR: the option '--run-config' is required but missing" << std::endl;
        }
        std::cout << "Available options:" << std::endl;
        std::cout << " " << std::endl;
        std::cout << "  -h [ --help ]          produce help message" << std::endl;
        std::cout << "  -c [ --run-config ] arg    The main config file (TOML format)." << std::endl;
        std::cout << "  -b [ --batch ] arg     Set to 1, if running in batch mode." << std::endl;
        return -1;
    }

    const auto colors = Color::Colors(batch_mode);

    // print pretty stuff
    if (!batch_mode)
    {
        system("clear");
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << " " << std::endl;
        std::cout << colors.acqua << "        ███    ███ ██    ██ ███████ ██  ██████ " << colors.def << std::endl;
        std::cout << colors.acqua << "        ████  ████ ██    ██ ██      ██ ██      " << colors.def << std::endl;
        std::cout << colors.acqua << "        ██ ████ ██ ██    ██ ███████ ██ ██      " << colors.def << std::endl;
        std::cout << colors.acqua << "        ██  ██  ██ ██    ██      ██ ██ ██      " << colors.def << std::endl;
        std::cout << colors.acqua << "        ██      ██  ██████  ███████ ██  ██████ " << colors.def << std::endl;
    }

    std::cout << " " << std::endl;
    std::cout << " " << std::endl;
    std::cout << colors.acqua << "        MUSiC - Model Unspecific Search in CMS" << colors.def << std::endl;
    std::cout << colors.acqua
              << emojicpp::emojize("              :signal_strength: Run2 - Ultra Legacy :signal_strength:")
              << colors.def << std::endl;
    std::cout << " " << std::endl;

    std::cout << " " << std::endl;
    std::cout << colors.yellow << "Checking run configuration [" << run_config_file << "] ..." << colors.def
              << std::endl;
    std::cout << " " << std::endl;

    const auto configuration = TaskConfiguration(run_config_file);

    std::cout << " " << std::endl;
    std::cout << colors.yellow << "Preparing output buffer ..." << colors.def << std::endl;
    prepare_output_buffer(configuration);
    std::cout << " " << std::endl;

    // Init the run config
    std::cout << " " << std::endl;
    std::cout << colors.green << "Initializing ..." << colors.def << std::endl;
    std::cout << " " << std::endl;

    std::cout << colors.def << "[ Initializing ] Run Lumi Filter ..." << colors.def << std::endl;
    auto run_lumi_filter = RunLumiFilter(configuration.golden_json_file);

    std::cout << colors.def << "[ Initializing ] PU corrections ..." << colors.def << std::endl;
    auto pu_weight = Corrector("PU"sv, configuration.year, configuration.is_data);

    std::cout << colors.def << "[ Initializing ] Rochester Muon Momentum Corrections ..." << colors.def << std::endl;
    auto rochester_corrections = Corrector("MuonLowPt"sv, configuration.year, configuration.is_data);

    std::cout << colors.def << "[ Initializing ] Jet Corrections ..." << colors.def << std::endl;
    auto jet_corrections = JetCorrector(configuration.year, configuration.era, configuration.is_data);

    std::cout << colors.def << "[ Initializing ] Muon SFs ..." << colors.def << std::endl;
    auto muon_sf_reco = Corrector("MuonReco"sv, configuration.year, configuration.is_data);
    auto muon_sf_id_low_pt = Corrector("MuonIdLowPt"sv, configuration.year, configuration.is_data);
    auto muon_sf_id_high_pt = Corrector("MuonIdHighPt"sv, configuration.year, configuration.is_data);
    auto muon_sf_iso_low_pt = Corrector("MuonIsoLowPt"sv, configuration.year, configuration.is_data);
    auto muon_sf_iso_high_pt = Corrector("MuonIsoHighPt"sv, configuration.year, configuration.is_data);

    std::cout << colors.def << "[ Initializing ] Electron SFs ..." << colors.def << std::endl;
    auto electron_sf = ElectronSFCorrector(configuration.year, configuration.is_data);

    std::cout << colors.def << "[ Initializing ] Photon SFs ..." << colors.def << std::endl;
    auto photon_id_sf = PhotonSFCorrector("PhotonID", configuration.year, configuration.is_data);
    auto photon_pixel_seed_sf = PhotonSFCorrector("PixelSeed", configuration.year, configuration.is_data);

    std::cout << colors.def << "[ Initializing ] BTag SFs ..." << colors.def << std::endl;
    auto btag_sf = BTagSFCorrector(configuration.year, configuration.is_data);

    std::cout << colors.def << "[ Initializing ] Trigger matchers ..." << colors.def << std::endl;
    std::map<std::string_view, TrgObjMatcher> matchers = make_trgobj_matcher(configuration.year, configuration.is_data);

    std::cout << colors.def << "[ Initializing ] LHAPDF Sets ..." << colors.def << std::endl;
    // initilize pdf sets for fallback cases ...
    // NNPDF31_nnlo_as_0118_hessian
    int lha_id_fallback = 304400;
    int lha_size = 101;

    // Compute the PDF weight for this event using NNPDF31_nnlo_as_0118_hessian (304400) and divide the
    // new weight by the weight from the PDF the event was produced with.
    std::tuple<std::vector<std::unique_ptr<LHAPDF::PDF>>, std::unique_ptr<LHAPDF::PDF>, std::unique_ptr<LHAPDF::PDF>>
        default_pdf_sets;
    std::get<0>(default_pdf_sets).reserve(101);
    for (int idx = lha_id_fallback; idx < (lha_id_fallback + lha_size); idx++)
    {
        std::get<0>(default_pdf_sets).push_back(std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(lha_id_fallback)));
    }

    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0120 (319500) and divide the new
    // weight by the weight from the PDF the event was produced with.
    std::get<1>(default_pdf_sets) = std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(319500));

    // Compute the Alpha_S weight for this event using NNPDF31_nnlo_as_0116 (319300) and divide the new
    // weight by the weight from the PDF the event was produced with.
    std::get<2>(default_pdf_sets) = std::unique_ptr<LHAPDF::PDF>(LHAPDF::mkPDF(319300));

    // read cross-sections files
    // std::cout << colors.def << "[ Initializing ] X-Sections ..." << colors.def << std::endl;
    // const auto x_sections = TOMLConfig::make_toml_config(configuration.x_section_file);

    // performance monitoring
    std::cout << colors.def << "[ Initializing ] Event counter and timer ..." << colors.def << std::endl;
    unsigned long event_counter = 0;
    double dTime1 = 0;

    // build outputs
    std::string output_file_name = configuration.is_crab_job
                                       ? "nano_music.root"
                                       : "nano_music_" + configuration.process + "_" + configuration.year_str + ".root";
    Outputs outputs(output_file_name);

    // create file chain and tree reader
    std::cout << colors.def << "Loading input file ..." << colors.def << std::endl;
    auto chain = TChain("Events");
    for (auto &&file : configuration.input_files)
    {
        chain.Add(file.c_str());
    }

    // create tree reader and add values and arrays
    auto tree_reader = TTreeReader(&chain);

    // event info
    ADD_VALUE_READER(run, UInt_t);
    // ADD_VALUE_READER(lumi, UInt_t);
    auto lumi = make_value_reader<UInt_t>(tree_reader, "luminosityBlock");
    // ADD_VALUE_READER(event_number, ULong64_t);
    auto event_number = make_value_reader<ULong64_t>(tree_reader, "event");
    ADD_VALUE_READER(Pileup_nTrueInt, float);
    ADD_VALUE_READER(genWeight, float);
    ADD_VALUE_READER(L1PreFiringWeight_Nom, float);
    ADD_VALUE_READER(L1PreFiringWeight_Up, float);
    ADD_VALUE_READER(L1PreFiringWeight_Dn, float);
    ADD_VALUE_READER(PV_npvsGood, int);
    ADD_VALUE_READER(Flag_goodVertices, bool);
    ADD_VALUE_READER(Flag_globalSuperTightHalo2016Filter, bool);
    ADD_VALUE_READER(Flag_HBHENoiseFilter, bool);
    ADD_VALUE_READER(Flag_HBHENoiseIsoFilter, bool);
    ADD_VALUE_READER(Flag_EcalDeadCellTriggerPrimitiveFilter, bool);
    ADD_VALUE_READER(Flag_BadPFMuonFilter, bool);
    ADD_VALUE_READER(Flag_BadPFMuonDzFilter, bool);
    ADD_VALUE_READER(Flag_eeBadScFilter, bool);
    ADD_VALUE_READER(Flag_ecalBadCalibFilter, bool);
    ADD_VALUE_READER(fixedGridRhoFastjetAll, float);
    ADD_VALUE_READER(HLT_IsoMu27, bool);
    ADD_VALUE_READER(HLT_IsoMu24, bool);
    ADD_VALUE_READER(HLT_IsoTkMu24, bool);
    ADD_VALUE_READER(HLT_Mu50, bool);
    ADD_VALUE_READER(HLT_TkMu50, bool);
    ADD_VALUE_READER(HLT_TkMu100, bool);
    ADD_VALUE_READER(HLT_OldMu100, bool);
    ADD_VALUE_READER(HLT_Ele27_WPTight_Gsf, bool);
    ADD_VALUE_READER(HLT_Ele35_WPTight_Gsf, bool);
    ADD_VALUE_READER(HLT_Ele32_WPTight_Gsf, bool);
    ADD_VALUE_READER(HLT_Photon200, bool);
    ADD_VALUE_READER(HLT_Photon175, bool);
    ADD_VALUE_READER(HLT_Ele115_CaloIdVT_GsfTrkIdT, bool);

    // Generator info
    ADD_VALUE_READER(Generator_binvar, float);
    ADD_VALUE_READER(Generator_scalePDF, float);
    ADD_VALUE_READER(Generator_weight, float);
    ADD_VALUE_READER(Generator_x1, float);
    ADD_VALUE_READER(Generator_x2, float);
    ADD_VALUE_READER(Generator_xpdf1, float);
    ADD_VALUE_READER(Generator_xpdf2, float);
    ADD_VALUE_READER(Generator_id1, int);
    ADD_VALUE_READER(Generator_id2, int);

    // // GenParticles
    // ADD_ARRAY_READER(GenPart_eta, float);
    // ADD_ARRAY_READER(GenPart_mass, float);
    // ADD_ARRAY_READER(GenPart_phi, float);
    // ADD_ARRAY_READER(GenPart_pt, float);
    // ADD_ARRAY_READER(GenPart_genPartIdxMother, int);
    // ADD_ARRAY_READER(GenPart_pdgId, int);
    // ADD_ARRAY_READER(GenPart_status, int);
    // ADD_ARRAY_READER(GenPart_statusFlags, int);

    // LHE info
    ADD_ARRAY_READER(LHEPdfWeight, float);
    ADD_ARRAY_READER(LHEScaleWeight, float);
    ADD_VALUE_READER(LHEWeight_originalXWGTUP, float);
    ADD_VALUE_READER(LHE_HT, Float_t);
    ADD_VALUE_READER(LHE_HTIncoming, Float_t);

    // LHE Particles
    ADD_ARRAY_READER(LHEPart_pt, float);
    ADD_ARRAY_READER(LHEPart_eta, float);
    ADD_ARRAY_READER(LHEPart_phi, float);
    ADD_ARRAY_READER(LHEPart_mass, float);
    ADD_ARRAY_READER(LHEPart_incomingpz, float);
    ADD_ARRAY_READER(LHEPart_pdgId, int);
    ADD_ARRAY_READER(LHEPart_status, int);

    // generator jets
    ADD_ARRAY_READER(GenJet_eta, float);
    // ADD_ARRAY_READER(GenJet_mass, float);
    ADD_ARRAY_READER(GenJet_phi, float);
    ADD_ARRAY_READER(GenJet_pt, float);

    // muons
    ADD_ARRAY_READER(Muon_pt, float);
    ADD_ARRAY_READER(Muon_eta, float);
    ADD_ARRAY_READER(Muon_phi, float);
    ADD_ARRAY_READER(Muon_tightId, bool);
    ADD_ARRAY_READER(Muon_highPtId, UChar_t);
    ADD_ARRAY_READER(Muon_pfRelIso04_all, float);
    ADD_ARRAY_READER(Muon_tkRelIso, float);
    ADD_ARRAY_READER(Muon_highPurity, bool);
    ADD_ARRAY_READER(Muon_tunepRelPt, float);

    // electrons
    ADD_ARRAY_READER(Electron_pt, float);
    ADD_ARRAY_READER(Electron_eta, float);
    ADD_ARRAY_READER(Electron_phi, float);
    ADD_ARRAY_READER(Electron_cutBased, int);
    ADD_ARRAY_READER(Electron_cutBased_HEEP, bool);
    ADD_ARRAY_READER(Electron_deltaEtaSC, float);
    ADD_ARRAY_READER(Electron_dEscaleDown, float);
    ADD_ARRAY_READER(Electron_dEscaleUp, float);
    ADD_ARRAY_READER(Electron_dEsigmaDown, float);
    ADD_ARRAY_READER(Electron_dEsigmaUp, float);
    // ADD_ARRAY_READER(Electron_eCorr, float);

    // photons
    ADD_ARRAY_READER(Photon_pt, float);
    ADD_ARRAY_READER(Photon_eta, float);
    ADD_ARRAY_READER(Photon_phi, float);
    ADD_ARRAY_READER(Photon_cutBased, int);
    ADD_ARRAY_READER(Photon_pixelSeed, bool);
    ADD_ARRAY_READER(Photon_isScEtaEB, bool);
    ADD_ARRAY_READER(Photon_isScEtaEE, bool);

    // taus
    ADD_ARRAY_READER(Tau_pt, float);
    ADD_ARRAY_READER(Tau_eta, float);
    ADD_ARRAY_READER(Tau_phi, float);

    // jets
    ADD_ARRAY_READER(Jet_pt, float);
    ADD_ARRAY_READER(Jet_eta, float);
    ADD_ARRAY_READER(Jet_phi, float);
    ADD_ARRAY_READER(Jet_mass, float);
    ADD_ARRAY_READER(Jet_jetId, int);
    ADD_ARRAY_READER(Jet_btagDeepFlavB, float);
    ADD_ARRAY_READER(Jet_hadronFlavour, int);
    ADD_ARRAY_READER(Jet_genJetIdx, int);
    ADD_ARRAY_READER(Jet_rawFactor, float);
    ADD_ARRAY_READER(Jet_area, float);

    // met
    ADD_VALUE_READER(MET_pt, float);
    ADD_VALUE_READER(MET_phi, float);

    // trgobjs
    ADD_ARRAY_READER(TrigObj_pt, float);
    ADD_ARRAY_READER(TrigObj_eta, float);
    ADD_ARRAY_READER(TrigObj_phi, float);
    ADD_ARRAY_READER(TrigObj_id, int);
    ADD_ARRAY_READER(TrigObj_filterBits, int);

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////       [ BEGIN ]      //////////////////////////////////////
    //////////////////////////////////////   loop over events   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    std::cout << " " << std::endl;
    std::cout << colors.green << "Starting Classification ..." << colors.def << std::endl;
    std::cout << " " << std::endl;

    std::cout << " " << std::endl;
    std::cout << colors.green << "Checking for LHE PDF Info ..." << colors.def << std::endl;
    std::cout << " " << std::endl;

    std::optional<std::pair<unsigned int, unsigned int>> lha_indexes =
        PDFAlphaSWeights::get_pdf_ids(tree_reader.GetTree());

    std::cout << colors.acqua << "Starting timer ..." << colors.def << std::endl;
    dTime1 = getCpuTime(); // Start Timer

    // launch event loop for Data or MC
    std::cout << colors.green << "\nLaunching event loop ..." << colors.def << std::endl;

    for (auto &&evt : tree_reader)
    {
        event_counter = evt;

        // clear outputs buffers
        outputs.clear_event_tree();

        // build event data
        auto event_analyzer =
            EventAnalyzer(configuration.is_data, configuration.year, outputs)
                // event info
                .set_event_info(NanoObjects::EventInfo(unwrap(run),                       //
                                                       unwrap(lumi),                      //
                                                       unwrap(event_number),              //
                                                       unwrap(Pileup_nTrueInt),           //
                                                       unwrap(genWeight, 1.),             //
                                                       unwrap(L1PreFiringWeight_Nom, 1.), //
                                                       unwrap(L1PreFiringWeight_Up, 1.),  //
                                                       unwrap(L1PreFiringWeight_Dn, 1.), //                           //
                                                       unwrap(PV_npvsGood),              //
                                                       unwrap(Flag_goodVertices),        //
                                                       unwrap(Flag_globalSuperTightHalo2016Filter),     //
                                                       unwrap(Flag_HBHENoiseFilter),                    //
                                                       unwrap(Flag_HBHENoiseIsoFilter),                 //
                                                       unwrap(Flag_EcalDeadCellTriggerPrimitiveFilter), //
                                                       unwrap(Flag_BadPFMuonFilter),                    //
                                                       unwrap(Flag_BadPFMuonDzFilter),                  //
                                                       unwrap(Flag_eeBadScFilter),                      //
                                                       unwrap(Flag_ecalBadCalibFilter),
                                                       unwrap(fixedGridRhoFastjetAll), //
                                                       unwrap(HLT_IsoMu27),            //
                                                       unwrap(HLT_IsoMu24),            //
                                                       unwrap(HLT_IsoTkMu24),          //
                                                       unwrap(HLT_Mu50),               //
                                                       unwrap(HLT_TkMu50),             //
                                                       unwrap(HLT_TkMu100),            //
                                                       unwrap(HLT_OldMu100),           //
                                                       unwrap(HLT_Ele27_WPTight_Gsf),  //
                                                       unwrap(HLT_Ele35_WPTight_Gsf),  //
                                                       unwrap(HLT_Ele32_WPTight_Gsf),  //
                                                       unwrap(HLT_Photon200),          //
                                                       unwrap(HLT_Photon175),          //
                                                       unwrap(HLT_Ele115_CaloIdVT_GsfTrkIdT)))
                // generator info
                .set_generator_info(NanoObjects::GeneratorInfo(unwrap(Generator_binvar),     //
                                                               unwrap(Generator_scalePDF),   //
                                                               unwrap(Generator_weight, 1.), //
                                                               unwrap(Generator_x1),         //
                                                               unwrap(Generator_x2),         //
                                                               unwrap(Generator_xpdf1),      //
                                                               unwrap(Generator_xpdf2),      //
                                                               unwrap(Generator_id1),        //
                                                               unwrap(Generator_id2)))       //

                // lhe info
                .set_lhe_info(NanoObjects::LHEInfo(unwrap(LHEPdfWeight),   //
                                                   unwrap(LHEScaleWeight), //
                                                   unwrap(LHEWeight_originalXWGTUP, 1.),
                                                   unwrap(LHE_HT),
                                                   unwrap(LHE_HTIncoming))) //

                // // gen particles
                // .set_gen_particles(NanoObjects::GenParticles(unwrap(GenPart_pt),
                //                                              unwrap(GenPart_eta),
                //                                              unwrap(GenPart_phi),
                //                                              unwrap(GenPart_mass),
                //                                              unwrap(GenPart_genPartIdxMother),
                //                                              unwrap(GenPart_pdgId),
                //                                              unwrap(GenPart_status),
                //                                              unwrap(GenPart_statusFlags)))

                // lhe particles
                .set_lhe_particles(NanoObjects::LHEParticles(unwrap(LHEPart_pt),
                                                             unwrap(LHEPart_eta),
                                                             unwrap(LHEPart_phi),
                                                             unwrap(LHEPart_mass),
                                                             unwrap(LHEPart_incomingpz),
                                                             unwrap(LHEPart_pdgId),
                                                             unwrap(LHEPart_status)))

                // GenJets
                .set_gen_jets(NanoObjects::GenJets(unwrap(GenJet_pt), unwrap(GenJet_eta), unwrap(GenJet_phi)))

                // muons
                .set_muons(NanoObjects::Muons(unwrap(Muon_pt),             //
                                              unwrap(Muon_eta),            //
                                              unwrap(Muon_phi),            //
                                              unwrap(Muon_tightId),        //
                                              unwrap(Muon_highPtId),       //
                                              unwrap(Muon_pfRelIso04_all), //
                                              unwrap(Muon_tkRelIso),
                                              unwrap(Muon_highPurity),
                                              unwrap(Muon_tunepRelPt)))
                // electrons
                .set_electrons(NanoObjects::Electrons(unwrap(Electron_pt),            //
                                                      unwrap(Electron_eta),           //
                                                      unwrap(Electron_phi),           //
                                                      unwrap(Electron_cutBased),      //
                                                      unwrap(Electron_cutBased_HEEP), //
                                                      unwrap(Electron_deltaEtaSC),
                                                      unwrap(Electron_dEscaleDown),
                                                      unwrap(Electron_dEscaleUp),
                                                      unwrap(Electron_dEsigmaDown),
                                                      unwrap(Electron_dEsigmaUp)))
                // photons
                .set_photons(NanoObjects::Photons(unwrap(Photon_pt),        //
                                                  unwrap(Photon_eta),       //
                                                  unwrap(Photon_phi),       //
                                                  unwrap(Photon_cutBased),  //
                                                  unwrap(Photon_pixelSeed), //
                                                  unwrap(Photon_isScEtaEB), //
                                                  unwrap(Photon_isScEtaEE)))
                // taus
                .set_taus(NanoObjects::Taus(unwrap(Tau_pt),  //
                                            unwrap(Tau_eta), //
                                            unwrap(Tau_phi)))
                // bjets
                .set_bjets(NanoObjects::BJets(unwrap(Jet_pt),  //
                                              unwrap(Jet_eta), //
                                              unwrap(Jet_phi),
                                              unwrap(Jet_mass),          //
                                              unwrap(Jet_jetId),         //
                                              unwrap(Jet_btagDeepFlavB), //
                                              unwrap(Jet_hadronFlavour),
                                              unwrap(Jet_genJetIdx),
                                              unwrap(Jet_rawFactor),
                                              unwrap(Jet_area)))
                // jets
                .set_jets(NanoObjects::Jets(unwrap(Jet_pt),  //
                                            unwrap(Jet_eta), //
                                            unwrap(Jet_phi),
                                            unwrap(Jet_mass),          //
                                            unwrap(Jet_jetId),         //
                                            unwrap(Jet_btagDeepFlavB), //
                                            unwrap(Jet_hadronFlavour),
                                            unwrap(Jet_genJetIdx),
                                            unwrap(Jet_rawFactor),
                                            unwrap(Jet_area)))
                // met
                .set_met(NanoObjects::MET({unwrap(MET_pt)}, //
                                          {0.},             //
                                          {unwrap(MET_phi)}))
                // trgobjs
                .set_trgobjs(NanoObjects::TrgObjs(unwrap(TrigObj_pt),  //
                                                  unwrap(TrigObj_eta), //
                                                  unwrap(TrigObj_phi), //
                                                  unwrap(TrigObj_id),  //
                                                  unwrap(TrigObj_filterBits)));

        event_analyzer = event_analyzer.set_pdf_alpha_s_weights(lha_indexes, default_pdf_sets)
                             .set_scale_weights()
                             .set_const_weights(outputs, pu_weight)
                             .generator_filter(outputs, configuration.process)
                             .run_lumi_filter(outputs, run_lumi_filter)
                             .npv_filter(outputs)
                             .met_filter(outputs)
                             .set_trigger_bits()
                             .trigger_filter(outputs)
                             .transform_muons("nominal")
                             .transform_electrons("nominal")
                             .transform_photons("nominal")
                             .transform_taus("nominal")
                             .transform_bjets_and_jets("nominal", jet_corrections)
                             .transform_met("nominal")
                             .object_selection()
                             .has_selected_objects_filter(outputs)
                             .trigger_match_filter(outputs, matchers)
                             .set_l1_pre_firing_SFs(outputs)
                             .set_muon_SFs(outputs,
                                           muon_sf_reco,
                                           muon_sf_id_low_pt,
                                           muon_sf_id_high_pt,
                                           muon_sf_iso_low_pt,
                                           muon_sf_iso_high_pt)
                             .set_electron_SFs(outputs, electron_sf)
                             .set_photon_SFs(outputs, photon_id_sf, photon_pixel_seed_sf)
                             .set_tau_SFs(outputs)
                             .set_jet_SFs(outputs)
                             .set_bjet_SFs(outputs, btag_sf)
                             .set_met_SFs(outputs)
                             .set_trigger_SFs(outputs)
                             .fill_event_content(outputs);

        // fill output event tree
        if (event_analyzer)
        {
            outputs.fill_event_tree();
        }

        // fmt::print("a\n");
        // process monitoring
        if (event_counter < 10 || (event_counter < 100 && event_counter % 10 == 0) ||
            (event_counter < 1000 && event_counter % 100 == 0) ||
            (event_counter < 10000 && event_counter % 1000 == 0) ||
            (event_counter >= 10000 && event_counter % 10000 == 0))
        {
            print_report(dTime1, event_counter, outputs.cutflow_histo);
            PrintProcessInfo();
        }
        // fmt::print("e\n");
    }

    std::cout << colors.green << "Event loop done ..." << colors.def << std::endl;

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////       [ END ]        //////////////////////////////////////
    //////////////////////////////////////   loop over events   //////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////

    std::cout << " " << std::endl;
    std::cout << colors.green << "Classification done ..." << colors.def << std::endl;
    std::cout << " " << std::endl;

    // show final performance report
    print_report(dTime1, event_counter, outputs.cutflow_histo, true);

    // writes data to disk
    std::cout << colors.yellow << "[ Finalizing ] Output file, cutflow histograms and event data trees ..."
              << colors.def << std::endl;
    outputs.write_data();

    PrintProcessInfo();

    // this is a dummy file, create to flag if the codde run all the way to the end
    // should be used by CRAB to confirm the completion of the task
    if (batch_mode)
    {
        fmt::print("Saving success_flag.out ...\n");
        std::ofstream success_flag("success_flag.out");
        success_flag << "Yay!";
        success_flag.close();
    }

    std::cout << colors.green << "\nDone ..." << colors.def << std::endl;
    return 0;
}
