#include "ClassFactory.hpp"
#include "TError.h"
#include "TROOT.h"
#include "TStyle.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/filesystem.hpp"
#include "boost/format.hpp"
#pragma GCC diagnostic pop

using namespace std::string_literals;

// will ready the eventview and build the ParticleMap lists (fourvectors (particles), scale_factors and matches )
auto getParticleLists(EventView &event_view)
    -> std::tuple<ParticleMap::ParticleMap_t, ParticleMap::ScaleFactorMap_t, ParticleMap::MatchMap_t>
{

    auto [particles_map, scalefactors_map, matches_map] = ParticleMap::make_empty_particle_maps();

    particles_map.at("Ele") = event_view.get_electrons();
    particles_map.at("Muon") = event_view.get_muons();
    particles_map.at("GammaEB") = event_view.get_photons();
    particles_map.at("Jet") = event_view.get_jets();
    particles_map.at("bJet") = event_view.get_jets();
    particles_map.at("MET") = event_view.get_met();

    scalefactors_map.at("Ele") = event_view.get_electrons_scalefactors();
    scalefactors_map.at("Muon") = event_view.get_muons_scalefactors();
    scalefactors_map.at("GammaEB") = event_view.get_photons_scalefactors();
    scalefactors_map.at("Jet") = event_view.get_jets_scalefactors();
    scalefactors_map.at("bJet") = event_view.get_jets_scalefactors();
    scalefactors_map.at("MET") = event_view.get_met_scalefactors();

    matches_map.at("Ele") = event_view.get_electrons_matches();
    matches_map.at("Muon") = event_view.get_muons_matches();
    matches_map.at("GammaEB") = event_view.get_photons_matches();
    matches_map.at("Jet") = event_view.get_jets_matches();
    matches_map.at("bJet") = event_view.get_jets_matches();
    matches_map.at("MET") = event_view.get_met_matches();

    return std::tuple<ParticleMap::ParticleMap_t, ParticleMap::ScaleFactorMap_t, ParticleMap::MatchMap_t>(
        particles_map, scalefactors_map, matches_map);
}

// Function to order physics object names
bool orderParticleTypes(std::string first, std::string second)
{

    std::map<std::string, int> orderMap;
    orderMap.emplace("Ele", 0);
    orderMap.emplace("EleEB", 1);
    orderMap.emplace("EleEE", 2);
    orderMap.emplace("Muon", 3);
    orderMap.emplace("Tau", 4);
    orderMap.emplace("Gamma", 6);
    orderMap.emplace("GammaEB", 7);
    orderMap.emplace("GammaEE", 8);
    orderMap.emplace("bJet", 9);
    orderMap.emplace("Jet", 10);
    orderMap.emplace("MET", 11);

    int first_pos = 99;
    if (orderMap.find(first) != orderMap.end())
    {
        first_pos = orderMap[first];
    }

    int second_pos = 99;
    if (orderMap.find(second) != orderMap.end())
    {
        second_pos = orderMap[second];
    }

    if (first_pos == second_pos)
    {
        return true;
    }
    else
    {
        return first_pos < second_pos;
    }
}

ClassFactory::ClassFactory(const bool is_data,
                           const double cross_section,
                           const double filter_efficiency,
                           const double k_factor,
                           const double lumi,
                           unsigned int const numPDFs,
                           const std::string &process,
                           const std::string &processOrder,
                           const std::string &processGroup,
                           const std::string outfilename,
                           const std::string hash,
                           const bool debug)
    : m_outfilename(outfilename),
      m_runHash(hash),
      m_debug(debug),

      m_cme(13000.),

      xsec(cross_section),
      fEff(filter_efficiency),
      kFac(k_factor),
      m_lumi(lumi),
      m_numPDFs(numPDFs),

      m_lastprocess(process),
      m_lastprocessOrder(processOrder),
      m_lastprocessGroup(processGroup),

      m_usePDF(true),
      m_useTrigger(true),
      m_use_prefire_check(false),

      m_syst_initialized(false),
      m_cutFlowUnweighted(TH1F("cutflow_unweighted", "cutflow_unweighted", 10, 0, 10)),
      m_cutFlowWeighted(TH1F("cutflow_weighted", "cutflow_weighted", 10, 0, 10)),
      m_cutFlowFilterUnweighted(TH1F("cutflow_filter_unweighted", "cutflow_filter_unweighted", 10, 0, 10)),
      m_cutFlowFilterWeighted(TH1F("cutflow_filter_weighted", "cutflow_filter_weighted", 10, 0, 10)),

      m_muo_use(false),
      m_ele_use(false),
      m_tau_use(false),
      m_gam_use(false),
      m_jet_use(true),
      m_met_use(true),

      m_ec_recOnly(true),
      m_ec_genOnly(false),
      m_ec_charge_use(false),

      m_ec_sumpt_min(0.),
      m_ec_minv_min(0.),
      m_ec_met_min(0.),

      m_num_listed_events_max(100),
      m_max_jet_ec(6),

      m_events_by_sumpt(),
      m_events_by_minv(),
      m_events_by_met(),

      m_fudge_sumpt(1.),
      m_fudge_minv(1.),
      m_fudge_met(1.),
      m_bin_size_min(10),

      m_useBJets(true),
      m_jet_bJets_algo("DeepJet=DeepFlavour"s),
      m_gam_regions_use(true),
      m_ele_regions_use(false),

      m_xsOrderErrorMap(CrossSectionOrderErrorMap()),

      m_data(is_data),

      m_differentialSystematics({}),
      m_constantSystematics({})
{
    m_fakeErrorMap = {
        {"Tau", 0.5},   //
        {"Ele", 0.5},   //
        {"Muon", 0.5},  //
        {"Jet", 0.},    //
        {"Gamma", 0.5}, //
        {"MET", 0.}     //
    };

    m_chargeErrorMap = {
        {"Tau", 0.5},   //
        {"Ele", 0.5},   //
        {"Muon", 0.5},  //
        {"Jet", 0.},    //
        {"Gamma", 0.5}, //
        {"MET", 0.}     //
    };

    if (m_ec_genOnly and m_ec_recOnly)
    {
        throw Tools::config_error("'m_ec_genOnly' and 'm_ec_recOnly' not allowed simultaneously!");
    }

    if (m_use_prefire_check)
    {
        std::cout << "Requesting simple prefire check for events" << std::endl;
    }

    // suppress root Info messages:
    gErrorIgnoreLevel = 1001;
    gROOT->SetStyle("Plain");

    m_cutFlowFilterWeighted.SetCanExtend(TH1::kAllAxes);

    m_cutFlowFilterUnweighted.SetCanExtend(TH1::kAllAxes);
}

// SYSTEMATICS ARE MISSING !!!

std::string ClassFactory::getQCDSystWeightName(std::string shift_type) const
{
    return "qcdWeight" + m_lastprocessGroup + "_" + shift_type;
}

void ClassFactory::FinishEventClasses(std::map<std::string, TEventClass *> &EvtClasses,
                                      std::set<std::string> &ProcessList,
                                      const std::string &Label)
{

    TFile OutputFile(Label.c_str(), "update");
    for (std::map<std::string, TEventClass *>::iterator Iter = EvtClasses.begin(); Iter != EvtClasses.end(); ++Iter)
    {
        // save this global process list (all physics processes contributing to any classes) in TEventClass
        Iter->second->setGlobalProcessList(ProcessList);
        Iter->second->setEventCount(eventCounts);
        Iter->second->setTotalEvents(totalEvents);
        Iter->second->setTotalEventsUnweighted(totalEventsUnweighted);
        if (!m_data)
        {
            if (m_usePDF)
                Iter->second->calculatePDFUncertainty(); // Perform PDF Reweighting
            Iter->second->scale(m_xsec * m_fEff * m_kFac);
        }
        if (Iter->second->isFilled())
        {
            Iter->second->addChangeLogEntry("Created");
            Iter->second->Write();
            WriteEventListToFile(Iter->second->GetName());
        }
    }
    OutputFile.Write();
    OutputFile.Close();
}

void ClassFactory::WriteEventListToFile(std::string const &ECname)
{
    std::vector<EventInfo> const event_list = GetListedEvents(ECname);

    std::ofstream outfile(("Event-lists/" + ECname + ".txt").c_str());
    // 'event_list' is always empty for 'Rec_Empty'.
    if (event_list.size() > 0)
    {
        outfile << "# dataset = " << event_list.at(0).dataset << std::endl;
    }
    outfile << std::string(160, '#') << std::endl;
    outfile << "#" << std::setw(12) << "sumpt:" << std::setw(13) << "minv:" << std::setw(11) << "met:" << std::setw(11)
            << "run:" << std::setw(15) << "lumi section:" << std::setw(16) << "event number:" << std::setw(72)
            << "pxlio file:" << std::setw(9) << "num:" << std::endl;
    outfile << std::string(160, '#') << std::endl;

    std::vector<EventInfo>::const_iterator event_it;
    int sumptMapSize = m_events_by_sumpt[ECname].size();
    int minvMapSize = m_events_by_minv[ECname].size();
    int counter = 0;
    outfile << "# Begin SumPt list" << std::endl;
    for (event_it = event_list.begin(); event_it != event_list.end(); ++event_it)
    {
        if ((float)counter / sumptMapSize == 1.)
            outfile << "# Begin Minv list" << std::endl;
        if ((float)counter / (minvMapSize + sumptMapSize) == 1.)
            outfile << "# Begin MET list" << std::endl;
        outfile << std::setw(13) << event_it->sumpt << "  " << std::setw(11) << event_it->minv << "  " << std::setw(9)
                << event_it->met << "  " << std::setw(9) << event_it->run << "  " << std::setw(13)
                << event_it->lumisection << "  " << std::setw(14) << event_it->eventnum << "  " << std::setw(70)
                << event_it->filename << "  " << std::setw(7) << event_it->eventnumpxlio << std::endl;
        counter++;
        ;
    }
    outfile.close();
}

std::vector<ClassFactory::EventInfo> ClassFactory::GetListedEvents(std::string const &ECname)
{
    std::vector<EventInfo> listed_events;

    std::map<double, EventInfo> const &sumptMap = m_events_by_sumpt[ECname];
    std::map<double, EventInfo> const &minvMap = m_events_by_minv[ECname];
    std::map<double, EventInfo> const &metMap = m_events_by_met[ECname];

    std::map<double, EventInfo>::const_iterator it;
    for (it = sumptMap.begin(); it != sumptMap.end(); it++)
    {
        listed_events.push_back(it->second);
    }

    for (it = minvMap.begin(); it != minvMap.end(); it++)
    {
        listed_events.push_back(it->second);
    }

    for (it = metMap.begin(); it != metMap.end(); it++)
    {
        listed_events.push_back(it->second);
    }

    return listed_events;
}

void ClassFactory::endJob()
{
    // cutflows in separated root file
    TFile *fout_cutflow = new TFile("cutflow.root", "RECREATE");
    if (!m_data && !eventCounts.empty())
    {
        std::string proc = eventCounts.begin()->first;
        const double totalXsec = xsec * fEff * kFac;
        m_cutFlowUnweighted.Scale(totalXsec);
        m_cutFlowWeighted.Scale(totalXsec);
        m_cutFlowFilterUnweighted.Scale(totalXsec);
        m_cutFlowFilterWeighted.Scale(totalXsec);
    }
    m_cutFlowUnweighted.Write();
    m_cutFlowWeighted.Write();
    m_cutFlowFilterUnweighted.Write();
    m_cutFlowFilterWeighted.Write();
    fout_cutflow->Close();
    delete fout_cutflow;

    // build a set of all analyzed processes
    std::set<std::string> ProcessList;
    for (std::map<std::string, unsigned int>::const_iterator proc_num = eventCounts.begin();
         proc_num != eventCounts.end();
         ++proc_num)
    {
        ProcessList.insert(proc_num->first);
    }

    WriteAnalysisInfoToFile(ProcessList);

    std::string Label;

    if (m_outfilename.empty())
    {
        Label = "EC";
        for (std::set<std::string>::iterator Iter = ProcessList.begin(); Iter != ProcessList.end(); Iter++)
        {
            std::string process = (*Iter);
            std::cout << " Found Process " << process << std::endl;
            if (Label.length() < 45)
            {
                Label += "_";
                Label += process;
            }
        }
    }
    else
    {
        Label = m_outfilename;
    }
    if (!boost::algorithm::ends_with(Label, ".root"))
    {
        Label += ".root";
    }
    FinishEventClasses(_recEventClasses, ProcessList, Label);
    FinishEventClasses(_recInclEventClasses, ProcessList, Label);
    FinishEventClasses(_recJetInclEventClasses, ProcessList, Label);
    if (!m_data)
    {
        FinishEventClasses(_genEventClasses, ProcessList, Label);
        FinishEventClasses(_genInclEventClasses, ProcessList, Label);
        FinishEventClasses(_genJetInclEventClasses, ProcessList, Label);
    }
    for (std::map<std::string, TEventClass *>::iterator Iter = _recEventClasses.begin(); Iter != _recEventClasses.end();
         Iter++)
    {
        if (Iter->second != 0)
        {
            delete Iter->second;
        }
    }
    for (std::map<std::string, TEventClass *>::iterator Iter = _recInclEventClasses.begin();
         Iter != _recInclEventClasses.end();
         Iter++)
    {
        if (Iter->second != 0)
        {
            delete Iter->second;
        }
    }
    for (std::map<std::string, TEventClass *>::iterator Iter = _recJetInclEventClasses.begin();
         Iter != _recJetInclEventClasses.end();
         Iter++)
    {
        if (Iter->second != 0)
        {
            delete Iter->second;
        }
    }
    if (!m_data)
    {
        for (std::map<std::string, TEventClass *>::iterator Iter = _genEventClasses.begin();
             Iter != _genEventClasses.end();
             Iter++)
        {
            if (Iter->second != 0)
            {
                delete Iter->second;
            }
        }
        for (std::map<std::string, TEventClass *>::iterator Iter = _genInclEventClasses.begin();
             Iter != _genInclEventClasses.end();
             Iter++)
        {
            if (Iter->second != 0)
            {
                delete Iter->second;
            }
        }
        for (std::map<std::string, TEventClass *>::iterator Iter = _genJetInclEventClasses.begin();
             Iter != _genJetInclEventClasses.end();
             Iter++)
        {
            if (Iter->second != 0)
            {
                delete Iter->second;
            }
        }
    }
}

void ClassFactory::WriteAnalysisInfoToFile(std::set<std::string> const &processList)
{
    // Use boost to get ISO formatted date and time.
    using namespace boost::posix_time;
    std::string const &isoTime = to_iso_extended_string(second_clock::local_time());

    // Create text file to store the info about the analyzed events.
    // In the main() function we cd into the output directory, so just store it
    // in "here".
    std::string const outFilePath(m_outfilename + "_Analyzed.txt");
    std::ofstream outFile(outFilePath.c_str(), std::ofstream::out);

    outFile << "# Creation time: " << isoTime << std::endl;
    outFile << std::endl;
    outFile << "# This file is meant to be easy to parse." << std::endl;
    outFile << std::endl;
    outFile << "################################################################################" << std::endl;
    outFile << "# The variables used below are defined as follows:" << std::endl;
    outFile << "# N = Event count, i.e. number of all events processed in this classification" << std::endl;
    outFile << "#     (no weights, simply count)." << std::endl;
    outFile << "# U = Sum of generator weights, but no PU weights [should be = N for data]." << std::endl;
    outFile << "# W = Sum of all weights [should be = N for data]." << std::endl;
    outFile << "# F = Event count after all filters (no weights, simply count)." << std::endl;
    outFile << "# f = Sum of generator weights of all events passing all filters" << std::endl;
    outFile << "#     [should be = F for data]." << std::endl;
    outFile << "# w = Sum of all weights of events passing all filters [should be = F for data]." << std::endl;
    outFile << "# R = Rec event count after all filters and selections" << std::endl;
    outFile << "#     (no weights, simply count) [only filled if not EventClass.GenOnly = 1]." << std::endl;
    outFile << "# r = Sum of all weights of Rec events passing all filters and selections" << std::endl;
    outFile << "#     [only filled if not EventClass.GenOnly = 1]." << std::endl;
    outFile << "# G = Gen event count after all filters and selections" << std::endl;
    outFile << "#     (no weights, simply count) [only filled if not EventClass.RecOnly = 1]." << std::endl;
    outFile << "# g = Sum of all weights of Gen events passing all filters and selections." << std::endl;
    outFile << "#     [only filled if not EventClass.RecOnly = 1]." << std::endl;
    outFile << "# a = R/N (in %) [only filled if not EventClass.GenOnly = 1]." << std::endl;
    outFile << "# b = r/W (in %) [only filled if not EventClass.GenOnly = 1]." << std::endl;
    outFile << "# c = G/N (in %) [only filled if not EventClass.RecOnly = 1]." << std::endl;
    outFile << "################################################################################" << std::endl;

    // In general, more than one process (i.e. MC sample) can be analyzed in one
    // classification job, but in most use cases there will be only one process.
    std::set<std::string>::const_iterator proc;
    for (proc = processList.begin(); proc != processList.end(); ++proc)
    {
        std::string const &thisProc = *proc;

        outFile << std::endl;
        outFile << "# Analyzed process '" << thisProc << "':" << std::endl;
        outFile << std::endl;

        unsigned int const N = eventCounts[thisProc];
        double const U = totalEventsUnweighted[thisProc];
        double const W = totalEvents[thisProc];
        unsigned int const F = numEventsFiltered[thisProc];
        double const f = totalEventsUnweightedFiltered[thisProc];
        double const w = totalEventsWeightedFiltered[thisProc];
        unsigned int const R = recTotalEventsUnweighted[thisProc];
        double const r = recTotalEventsWeighted[thisProc];
        unsigned int const G = genTotalEventsUnweighted[thisProc];
        double const g = genTotalEventsWeighted[thisProc];

        boost::format fmt("%.10g\n");
        boost::format fmtp("%.5g %%\n");

        outFile << "N = " << fmt % N;
        outFile << "U = " << fmt % U;
        outFile << "W = " << fmt % W;
        outFile << "F = " << fmt % F;
        outFile << "f = " << fmt % f;
        outFile << "w = " << fmt % w;

        // m_ec_genOnly and m_ec_recOnly cannot be both true at the same time.
        if (not m_ec_genOnly)
        {
            outFile << "R = " << fmt % R;
            outFile << "r = " << fmt % r;
        }

        if (not m_ec_recOnly)
        {
            outFile << "G = " << fmt % G;
            outFile << "g = " << fmt % g;
        }

        if (not m_ec_genOnly)
        {
            outFile << "a = " << fmtp % ((double(R) / double(N)) * 100.0);
            outFile << "b = " << fmtp % ((r / W) * 100.0);
        }

        if (not m_ec_recOnly)
        {
            outFile << "c = " << fmtp % ((G / double(N)) * 100.0);
        }

        outFile << std::flush;
    }
    outFile.close();

    if (m_debug > 0)
    {
        std::cerr << std::endl;
        std::cerr << "[INFO] (EventClassFactory):" << std::endl;
        std::cerr << "Written event numbers into file: '" << outFilePath << "'.";
        std::cerr << std::endl << std::endl;
    }
}

// Strip out extension sample strings from process name
void ClassFactory::prepareProcessName(std::string &proc)
{
    // strip out extension sample strings
    std::string ext_delimiter = "_ext";
    size_t pos = proc.find(ext_delimiter);
    if (pos != std::string::npos)
    {
        // +1 for extension number
        proc.erase(pos, ext_delimiter.length() + 1);
    }
}

// we left it out, initialized in constructor
// This method searches for the entry for the given process in the XSec file and throws an exception if not found!
// void ClassFactory::CheckForCrossSection()
//{
//}

void ClassFactory::fillFilterCutFlow(const double weight)
{
    m_cutFlowFilterUnweighted.Fill("All", 1.);
    m_cutFlowFilterWeighted.Fill("All", weight);
}

void ClassFactory::fillCutFlow(Event &event, const double weight)
{
    auto event_view = event.get_view("nominal");

    m_cutFlowUnweighted.Fill("all", 1.);
    m_cutFlowWeighted.Fill("all", weight);

    if (event_view.get_Veto())
    {
        return;
    }

    m_cutFlowUnweighted.Fill("Veto", 1.);
    m_cutFlowWeighted.Fill("Veto", weight);

    if (!event_view.get_trigger_accept())
    {
        return;
    }

    m_cutFlowUnweighted.Fill("trigger_accept", 1.);
    m_cutFlowWeighted.Fill("trigger_accept", weight);

    if (!event_view.get_filter_accept())
    {
        return;
    }

    m_cutFlowUnweighted.Fill("filter_accept", 1.);
    m_cutFlowWeighted.Fill("filter_accept", weight);

    if (!event_view.get_generator_accept())
    {
        return;
    }

    m_cutFlowUnweighted.Fill("generator_accept", 1.);
    m_cutFlowWeighted.Fill("generator_accept", weight);

    if (!event_view.get_topo_accept())
    {
        return;
    }

    m_cutFlowUnweighted.Fill("topo_accept", 1.);
    m_cutFlowWeighted.Fill("topo_accept", weight);
}

void ClassFactory::fillCutFlow(const double sumPt,
                               const double invMass,
                               const double met,
                               const int numPart,
                               const std::unordered_map<std::string, int> &countMap,
                               const double weight)
{
    if (sumPt < m_ec_sumpt_min)
    {
        return;
    }
    m_cutFlowUnweighted.Fill("sumPt_cut", 1.);
    m_cutFlowWeighted.Fill("sumPt_cut", weight);
    if (numPart > 1 and invMass < m_ec_minv_min)
    {
        return;
    }
    m_cutFlowUnweighted.Fill("invMass_cut", 1.);
    m_cutFlowWeighted.Fill("invMass_cut", weight);
    if (countMap.at("MET") > 0 and met < m_ec_met_min)
    {
        return;
    }
    m_cutFlowUnweighted.Fill("met_cut", 1.);
    m_cutFlowWeighted.Fill("met_cut", weight);
}

void ClassFactory::analyseEvent(Event &event)
{

    // store current event data
    // this struct is defined in TEventClass.hpp
    EventInfo event_info;
    event_info.run = event.get_view("nominal").get_Run();
    event_info.lumisection = event.get_view("nominal").get_LumiSection();
    event_info.eventnum = event.get_view("nominal").get_EventNum();
    event_info.dataset = event.get_view("nominal").get_Dataset();
    event_info.filename = event.get_view("nominal").get_Filename();
    event_info.eventnumpxlio = event.get_view("nominal").get_EventNumPxlio();
    event_info.prefire_weight = 1.;
    event_info.prefire_weight_up = 1.;
    event_info.prefire_weight_down = 1.;
    if (!m_data)
    {
        event_info.central_weight = event.get_view("nominal").get_genWeight();
        event_info.pileup = event.get_view("nominal").get_PUWeight();
        event_info.pileup_up = event.get_view("nominal").get_PUWeightUp();
        event_info.pileup_down = event.get_view("nominal").get_PUWeightDown();

        event_info.process = event.get_view("nominal").get_Process();
        event_info.has_scale_variation = false;
        if (not(event.get_view("nominal").get_scale_variation().empty()) and
            int(event.get_view("nominal").get_scale_variation_n()) != 0)
        {
            event_info.has_scale_variation = true;
            event_info.qcd_scale = event.get_view("nominal").get_scale_variation();
        }
        event_info.pdf_weights = event.get_view("nominal").get_pdf_weights();
        event_info.as_weights = event.get_view("nominal").get_as_weights();
        event_info.Global_ScalefactorError = event.get_view("nominal").get_Global_ScalefactorError();
    }
    else
    {
        event_info.central_weight = 1.;
        event_info.process = "";
        event_info.qcd_scale = "";
        event_info.pileup = 1.;
        event_info.pileup_up = 1.;
        event_info.pileup_down = 1.;
        event_info.pdf_weights = {};
        event_info.as_weights = {};
        event_info.Global_ScalefactorError = 1;
    }

    // by default, the event weight is 1
    double event_weight = 1;
    double pileup_weight = 1;

    std::string proc = event.get_view("nominal").get_Process();
    prepareProcessName(proc);
    double total_event_weight = event_weight * pileup_weight * event_info.prefire_weight;
    if (!m_data)
    {
        // CheckForCrossSection(proc);
        // we need to initialize some systematic info after the first event was loaded to determine, e.g.
        // the current process group
        if (!m_syst_initialized)
        {
            // addConstantSystematics();
            m_syst_initialized = true;
        }

        // should be done during preprocessing ...
        // m_pdfTool.setPDFWeights(GenEvtView);

        if (event.get_view("nominal").get_prefiring_scale_factor())
        {
            event_info.prefire_weight = event.get_view("nominal").get_prefiring_scale_factor();
            event_info.prefire_weight_up = event.get_view("nominal").get_prefiring_scale_factor_up();
            event_info.prefire_weight_down = event.get_view("nominal").get_prefiring_scale_factor_down();
        }

        // set the event weight to the actual value
        event_weight = event.get_view("nominal").get_genWeight();
        pileup_weight = event.get_view("nominal").get_PUWeight();
        total_event_weight = event_weight * pileup_weight * event_info.prefire_weight;

        // Fill systematics
        for (auto &&[view, eventview] : event)
        {
            if (view != "nominal")
            {
                Fill(eventview, total_event_weight, event_info, std::string(view));
            }
        }
    }
    // count this event, no matter if accepted or not
    eventCounts[proc] += 1;
    totalEvents[proc] += (total_event_weight);
    totalEventsUnweighted[proc] += event_weight;

    bool filter_accept = event.get_view("nominal").get_filter_accept();
    if (filter_accept)
    {
        numEventsFiltered[proc]++;
        totalEventsUnweightedFiltered[proc] += event_weight;
        totalEventsWeightedFiltered[proc] += (total_event_weight);
    }

    // Fill cuflow part 1
    fillCutFlow(event, total_event_weight);
    fillFilterCutFlow(total_event_weight);

    // fill this event
    Fill(event.get_view("nominal"), total_event_weight, event_info);
}

void ClassFactory::FillInclusiveRecursive(std::map<std::string, TEventClass *> &inclEventClasses,
                                          std::vector<std::string> partNameVector,
                                          std::string Type,
                                          std::string Process,
                                          double const weight,
                                          EventInfo &event_info,
                                          int iterator,
                                          std::unordered_map<std::string, int> recCountMap,
                                          std::unordered_map<std::string, int> refCountMap,
                                          ParticleMap &particleMap,
                                          std::string systName,
                                          std::string nameSuffix)
{

    std::string EventClassName;
    std::string partName = partNameVector.at(iterator);
    bool printIt = false;
    //~ for(auto countMap : refCountMap ) if( countMap.second > 1 ) printIt = true;
    for (int partCount = recCountMap[partName]; partCount <= refCountMap[partName]; partCount++)
    {
        recCountMap[partName] = partCount;
        if (printIt)
        {
            std::cout << std::endl << partName << " " << refCountMap[partName] << " " << recCountMap[partName];
            std::cout << std::endl << "refMap ";
            for (auto countMap : refCountMap)
                std::cout << countMap.second << countMap.first << " ";
            std::cout << std::endl << "recMap ";
            for (auto countMap : recCountMap)
                std::cout << countMap.second << countMap.first << " ";
        }
        if (partName == partNameVector.back())
        {
            bool allEmpty = true;
            // check if recCountMap is empty
            for (auto count : recCountMap)
            {
                if (count.second > 0)
                    allEmpty = false;
            }
            if (allEmpty)
                continue;
            // calculate and use charge only if asked to
            int iCharge = 0;
            // if (m_ec_charge_use)
            // {
            //     iCharge = abs(particleMap.getLeptonCharge(recCountMap));
            // }

            EventClassName = TEventClass::calculateEventClass(
                                 recCountMap, std::unordered_map<std::string, int>(), orderParticleTypes) +
                             nameSuffix;

            // Get the histo from the map per inclusive rec-Eventclass
            if (!hasTooManyJets(recCountMap))
            {
                TEventClass *InclEventClasstoFill = inclEventClasses[EventClassName];
                if (InclEventClasstoFill == 0 && !hasTooManyJets(recCountMap))
                {
                    InclEventClasstoFill = InitTEventClass(Type,
                                                           EventClassName,
                                                           particleMap,
                                                           recCountMap,
                                                           iCharge, //   !!!
                                                           true);
                    inclEventClasses[EventClassName] = InclEventClasstoFill;
                }
                if (m_data)
                {
                    // data mode, so there are no weights at all
                    FillEventClass(InclEventClasstoFill, event_info, Process, particleMap, recCountMap);
                }
                else
                {
                    FillEventClass(
                        InclEventClasstoFill, event_info, Process, particleMap, recCountMap, weight, systName);
                }
            }
        }
        else
        { // we are not at the deepest level yet
            FillInclusiveRecursive(inclEventClasses,
                                   partNameVector,
                                   Type,
                                   Process,
                                   weight,
                                   event_info,
                                   iterator + 1,
                                   recCountMap,
                                   refCountMap,
                                   particleMap,
                                   systName,
                                   nameSuffix);
        }
    }
}

// Create TEventClass pointer with all needed input. The deletion takes place in
// the destructor of EventClassFactory.
// Only some of the variables (type, name, number of particles, etc.) change
// from class to class. Some other variables are computed from these variables
// (bin edges, etc.). This wrapper function does this calculation, because they
// are always the same for all kinds of EventClasses (exclusive, inclusive,
// empty).
TEventClass *ClassFactory::InitTEventClass(std::string const &Type,
                                           std::string const &ECName,
                                           ParticleMap &particleMap,
                                           std::unordered_map<std::string, int> countMap,
                                           int const absCharge,
                                           bool const inclusiveEC) const
{
    // Calculate bin edges, depending on resolution.
    std::vector<double> const sumptBins =
        particleMap.getBinLimits("SumPt", countMap, 0, m_cme, m_bin_size_min, m_fudge_sumpt);

    std::vector<double> const minvBins =
        particleMap.getBinLimits("InvMass", countMap, 0, m_cme, m_bin_size_min, m_fudge_sumpt);

    std::vector<double> const metBins =
        particleMap.getBinLimits("MET", countMap, 0, m_cme, m_bin_size_min, m_fudge_sumpt);

    // Compute the minimal possible SumPt and MET from the set of particles in
    // this EventClass.
    // This is needed for the fill-up in MISv2.
    double sumPtMin = 0.0;
    double metMin = 0.0;
    double invMassMin = 0.0;

    // Fill all bin borders in map entries decide which histograms are
    // booked during init
    std::map<std::string, std::vector<double>> distTypeBins;
    distTypeBins.emplace("SumPt", sumptBins);
    distTypeBins.emplace("InvMass", minvBins);
    if (countMap["MET"] > 0 and ECName.find("Empty") == std::string::npos)
        distTypeBins.emplace("MET", metBins);
    // Fill topo and config requirements for all distTypes in to maps
    std::map<std::string, double> distTypMins;
    distTypMins.emplace("SumPt", sumPtMin);
    distTypMins.emplace("InvMass", invMassMin);
    distTypMins.emplace("MET", metMin);
    std::map<std::string, double> distTypeMinsRequire;
    distTypeMinsRequire.emplace("SumPt", m_ec_sumpt_min);
    distTypeMinsRequire.emplace("InvMass", m_ec_minv_min);
    distTypeMinsRequire.emplace("MET", m_ec_met_min);

    //~ std::cout << "init class "  << ECName << std::endl;
    // Tell EventClassFactory what systematics should be expected
    std::set<std::string> systNames;
    for (auto &&syst_name : m_differentialSystematics)
    {
        systNames.emplace(syst_name);
    }
    for (auto &&syst_name : m_constantSystematics)
    {
        systNames.emplace(syst_name);
    }

    std::cout << "Adding new class " << ECName << std::endl;
    return new TEventClass(Type,
                           ECName,
                           m_runHash,
                           m_data,
                           m_cme,
                           countMap,
                           m_useBJets,
                           distTypeBins,
                           m_ec_charge_use,
                           absCharge,
                           inclusiveEC,
                           distTypMins,
                           m_numPDFs,
                           distTypeMinsRequire,
                           m_lumi,
                           systNames);
}

void ClassFactory::FillEventClass(TEventClass *EventClassToFill,
                                  EventInfo &event_info,
                                  std::string const &process,
                                  ParticleMap &particleMap,
                                  std::unordered_map<std::string, int> &countMap,
                                  double const eventWeight,
                                  std::string systName)
{
    // What kind of EventClass are we dealing with?
    std::string const ECtype(EventClassToFill->getType()); // Gen or Rec
    std::string const ECname(EventClassToFill->GetName());
    // special treatment for current way to handle b jets

    bool minimalFill = false;
    // check if we want to fill pdfs and constant systematics ( not data or differential systematic)
    if (m_data or !systName.empty())
        minimalFill = true;

    unsigned int const numPart = EventClassToFill->getTotalNumECItems();

    double sumPt = 0.0;
    double MET = 0.0;
    double scale_factor = 1.0;
    double scale_factor_syst_error = 0.0;
    double Minv = 0.0;

    // Use pxl::Particle to add up all particle four-vectors.
    // pxl::Particle vec_sum = pxl::Particle();
    // auto vec_sum = Math::PtEtaPhiMVector();

    // create empty maps
    // for pxl particle vectors with reduced number of parts for inclusive classes
    // std::map<std::string, std::vector<pxl::Particle *>> realParticleMap;
    // for fake counting
    std::unordered_map<std::string, int> fakeMap;
    // std::unordered_map<std::string, int> chargeFakeMap;

    // Fill all considered particle types to the event distributions (distTypes)
    sumPt = particleMap.getSumPt(countMap);
    MET = particleMap.getMET(countMap);
    Minv = particleMap.getMass(countMap);
    if (ECtype == "Rec" and not minimalFill)
    {
        fakeMap = particleMap.getFakeMap(countMap);
        // chargeFakeMap = particleMap.getChargeFakeMap(countMap);
    }

    // Final event weight (always = 1 for data).
    double weight = 1.0;

    if (not m_data)
    {
        scale_factor = particleMap.getScaleFactor(countMap);
        scale_factor_syst_error = particleMap.getScaleFactorSystError(countMap);
        weight *= eventWeight;
        weight *= scale_factor;
    }

    double weight_fake_error = 0.0;
    if (not minimalFill)
    {
        for (auto errorPair : m_fakeErrorMap)
        {
            weight_fake_error += std::pow(errorPair.second * fakeMap[errorPair.first], 2);
        }
        weight_fake_error = sqrt(weight_fake_error);
    }

    // Compute weight for charge misassignment (MC only):
    double weight_charge_error = 0.0;
    // if (m_ec_charge_use and not minimalFill)
    // {
    //     for (auto errorPair : m_chargeErrorMap)
    //     {
    //         weight_charge_error += std::pow(errorPair.second * chargeFakeMap[errorPair.first], 2);
    //     }
    //     weight_charge_error = sqrt(weight_fake_error);
    // }

    // Check if the map of processes and process groups needs to be updated
    EventClassToFill->addToProcessGroupMap(process, m_lastprocessGroup);

    // Fill cut flow part 2
    if (systName == "" and not EventClassToFill->isInclusive())
    {
        fillCutFlow(sumPt, Minv, MET, numPart, countMap, eventWeight);
    }

    // If any of the required cuts it not fulfilled, do not fill the event!
    // Check Minv only if we have more that one particle (otherwise Minv is the
    // particle mass).
    // Check MET only if MET is present.
    if (sumPt < m_ec_sumpt_min)
        return;
    if (numPart > 1 and Minv < m_ec_minv_min)
        return;
    if (countMap["MET"] > 0 and MET < m_ec_met_min)
        return;

    // Compute "real" resolution of this event:
    double const sumptResReal = particleMap.getRealResolution(countMap);

    double const minvResReal = sumptResReal;

    // Compute "assumed" resolution of this event, i.e. with averaged pt:
    double const sumptRes = particleMap.getApproximateResolution(countMap);

    double const minvRes = sumptRes;

    // Fill values into maps, this allows more general
    // handling of both distributions and systematics
    std::map<std::string, double> values;
    values["SumPt"] = sumPt;
    values["InvMass"] = Minv;
    if (countMap["MET"] > 0)
        values["MET"] = MET;
    std::map<std::string, std::pair<double, double>> resolutionMap;
    resolutionMap["SumPt"] = std::make_pair(sumptResReal, sumptRes);
    resolutionMap["InvMass"] = std::make_pair(minvResReal, minvRes);
    if (m_data)
    {
        // Data, so no weights.
        EventClassToFill->Fill(process, values, resolutionMap);

        // Check if we have an interesting event here, if so, save it.
        if (numPart > 0 and CheckEventToList(sumPt, Minv, MET))
        {
            event_info.sumpt = sumPt;
            event_info.minv = Minv;
            event_info.met = MET;

            FillEventList(ECname, event_info);
        }
    }
    else if (!systName.empty())
    {
        // Fill differential systematic
        // the eventclass may not be filled yet.make sure the xs uncert is added
        // even though syst weight are not filled for differential systematics
        std::string xsUncertName = "xs" + m_lastprocessGroup + m_lastprocessOrder;
        if (!EventClassToFill->hasSystematic(xsUncertName))
        {
            EventClassToFill->addSystematic(xsUncertName);
        }
        EventClassToFill->FillDifferentialSystematic(process, values, weight, event_info.pdf_weights, systName);
    }
    else
    {
        // Fill standard MC and constant systematics
        std::map<std::string, double> systWeights;
        // fake
        systWeights["fakeUp"] = 1. + weight_fake_error;
        systWeights["fakeDown"] = 1. - weight_fake_error;
        systWeights["charge"] = 1 + weight_charge_error;
        systWeights["luminosity"] = 1 + event_info.Global_ScalefactorError;
        systWeights["scale_factor_syst"] = 1 + scale_factor_syst_error;
        std::pair<double, double> asUncerts = event_info.as_weights;

        // alphas uncert weights use too smaller variations compared to PDF4LHC recommendations
        // and should be scaled by a factor of 1.5, see slide 14 of
        // https://indico.cern.ch/event/459797/contributions/1961581/attachments/1181555/1800214/mcaod-Feb15-2016.pdf
        systWeights["alphasDown"] = 1 - 1.5 * (asUncerts.second - asUncerts.first) / 2;
        systWeights["alphasUp"] = 1 + 1.5 * (asUncerts.second - asUncerts.first) / 2;
        if (event_info.prefire_weight > 0)
        {
            systWeights["prefireUp"] = event_info.prefire_weight_up / event_info.prefire_weight;
            systWeights["prefireDown"] = event_info.prefire_weight_down / event_info.prefire_weight;
        }
        else
        {
            systWeights["prefireUp"] = 1.;
            systWeights["prefireDown"] = 1.;
        }

        systWeights["pileupDown"] = event_info.pileup_down / event_info.pileup;
        systWeights["pileupUp"] = event_info.pileup_up / event_info.pileup;

        // cross section uncertainty
        std::string xsUncertName = "xs" + m_lastprocessGroup + m_lastprocessOrder;
        systWeights[xsUncertName] = 1 + m_xsOrderErrorMap.at(m_lastprocessOrder);

        /// renormalization and factorization scales
        double QCDweightUp = 1;
        double QCDweightDown = 1;

        if (event_info.has_scale_variation)
        {

            std::string delimiter = ";";

            // get PDF weights
            std::vector<float> qcdScaleWeights;

            unsigned int pos1 = 0;
            unsigned int pos2 = 0;
            // first: string ending with number, second: string ending with delimiter
            while (pos2 != std::string::npos && pos1 != event_info.qcd_scale.length())
            {
                pos2 = event_info.qcd_scale.find(delimiter, pos1);
                qcdScaleWeights.push_back(std::stof(event_info.qcd_scale.substr(pos1, pos2 - pos1)) /
                                          event_info.central_weight);
                pos1 = pos2 + 1; // + delimiter.length();
            }

            /// find part of the string that gives up down for muR=muF=0.5 and muR=muF=2.
            QCDweightUp = qcdScaleWeights[4];
            QCDweightDown = qcdScaleWeights[8];
        }

        if (QCDweightUp > QCDweightDown)
        {
            systWeights[getQCDSystWeightName("Up")] = QCDweightUp;
            systWeights[getQCDSystWeightName("Down")] = QCDweightDown;
        }
        else
        {
            systWeights[getQCDSystWeightName("Up")] = QCDweightDown;
            systWeights[getQCDSystWeightName("Down")] = QCDweightUp;
        }

        // add cross section error correlationGroup if not added yet
        // this means no Event of this process / processGroup has been
        // added
        if (!EventClassToFill->hasSystematic(xsUncertName))
        {
            EventClassToFill->addSystematic(xsUncertName);
        }
        EventClassToFill->Fill(process, values, resolutionMap, weight, systWeights, event_info.pdf_weights);
    }
}

bool ClassFactory::CheckEventToList(double const sumPt, double const Minv, double const MET) const
{
    // Simply accept all events with anything in them, atm.
    if (sumPt > 0.0 or Minv > 0.0 or MET > 0.0)
        return true;
    return false;
}

void ClassFactory::FillEventList(std::string const &ECname, EventInfo const &event_info)
{
    // If the list is full, check for each list if the given event has a greater
    // sumpt, minv, met than that last element in the map, i.e. the one with the
    // lowest value of the kinematic variable.
    // Don't write the event info more than once, so return if it has been stored.
    std::map<double, EventInfo>::iterator last;

    std::map<double, EventInfo> &eventsBySumpt = m_events_by_sumpt[ECname];
    if (eventsBySumpt.size() < m_num_listed_events_max)
    {
        eventsBySumpt[event_info.sumpt] = event_info;
    }
    else
    {
        last = --eventsBySumpt.end();
        if (event_info.sumpt > last->first)
        {
            eventsBySumpt.erase(last);
            eventsBySumpt[event_info.sumpt] = event_info;
        }
    }

    std::map<double, EventInfo> &eventsByMinv = m_events_by_minv[ECname];
    if (eventsByMinv.size() < m_num_listed_events_max)
    {
        eventsByMinv[event_info.minv] = event_info;
    }
    else
    {
        last = --eventsByMinv.end();
        if (event_info.minv > last->first)
        {
            eventsByMinv.erase(last);
            eventsByMinv[event_info.minv] = event_info;
        }
    }

    std::map<double, EventInfo> &eventsByMET = m_events_by_met[ECname];
    if (eventsByMET.size() < m_num_listed_events_max)
    {
        eventsByMET[event_info.met] = event_info;
    }
    else
    {
        last = --eventsByMET.end();
        if (event_info.met > last->first)
        {
            eventsByMET.erase(last);
            eventsByMET[event_info.met] = event_info;
        }
    }
}

bool ClassFactory::hasTooManyJets(std::unordered_map<std::string, int> countMap)
{
    unsigned int njets = countMap["Jet"] + countMap["bJet"];
    return njets > m_max_jet_ec;
}

void ClassFactory::Fill(EventView &event_view, const double weight, EventInfo &event_info, std::string systName)
{
    // MET filters
    if (!event_view.get_filter_accept())
    {
        return;
    }

    // Cross cleaning of trigger streams
    if (event_view.get_Veto())
    {
        return;
    }

    // Trigger string and offline cuts fullfiled for one trigger
    if (!event_view.get_trigger_accept())
    {
        return;
    }

    // Generator cuts
    if (!event_view.get_generator_accept())
    {
        return;
    }

    // // simple prefiring check
    // if (m_use_prefire_check && !event_view.get_pass_prefiring_check())
    // {
    //     return;
    // }

    // std::string type = event_view.get_Type();
    // if ((type == "Gen" and m_ec_recOnly) or (type == "Rec" and m_ec_genOnly))
    // {
    //     return;
    // }

    // event with accepted topology?
    bool topo_accept = event_view.get_topo_accept();

    // method which creates and fills the EventClasses
    std::string processName = event_view.get_Process();

    // Delete 'ext' from process name ( via reference )
    prepareProcessName(processName);
    // EventClass specific

    auto [particleMapTmp, scaleFactorMapTmp, MatchesMapTmp] = ParticleMap::make_empty_particle_maps();

    if (topo_accept)
    {
        // Count this event only if it is accepted.
        if (systName.empty())
        {
            // if (type == "Gen")
            // {
            //     genTotalEventsWeighted[processName] += weight;
            //     genTotalEventsUnweighted[processName]++;
            // }
            // else
            // {
            recTotalEventsWeighted[processName] += weight;
            recTotalEventsUnweighted[processName]++;
            // }
        }

        // get a map containing only selected particles
        auto particle_lists = getParticleLists(event_view);
        particleMapTmp = std::get<0>(particle_lists);
        scaleFactorMapTmp = std::get<1>(particle_lists);
        MatchesMapTmp = std::get<2>(particle_lists);

        particleMapTmp.erase("GammaEE");
        scaleFactorMapTmp.erase("GammaEE");
        MatchesMapTmp.erase("GammaEE");
    }

    auto particleMap = ParticleMap(particleMapTmp, scaleFactorMapTmp, MatchesMapTmp);
    auto countMap = particleMap.getCountMap();

    // calculate the class name
    std::string excl_EC_name;
    std::string incl_empty_name;
    int absCharge = 0;
    // if (m_ec_charge_use)
    // {
    //     absCharge = abs(particleMap.getLeptonCharge(countMap));
    // }

    excl_EC_name =
        TEventClass::calculateEventClass(countMap, std::unordered_map<std::string, int>(), orderParticleTypes);
    incl_empty_name = TEventClass::calculateEventClass(std::unordered_map<std::string, int>()) + "+X";

    // introduced only to avoid too many changes in the interface
    std::string type = "Rec";

    // get the event class maps
    std::map<std::string, TEventClass *> &exclEventClasses = getReferenceExclusiveMap(type);
    std::map<std::string, TEventClass *> &inclEventClasses = getReferenceInclusiveMap(type);
    std::map<std::string, TEventClass *> &jetInclEventClasses = getReferenceJetInclusiveMap(type);

    // all accepted events are put in the Empty+X class
    TEventClass *incl_empty = inclEventClasses[incl_empty_name];
    if (incl_empty == 0)
    {
        // EventClass not existing, create it
        incl_empty =
            InitTEventClass(type, incl_empty_name, particleMap, std::unordered_map<std::string, int>(), 0.0, true);

        inclEventClasses[incl_empty_name] = incl_empty;
    }
    std::unordered_map<std::string, int> emptyMap = countMap;
    for (auto &pair : emptyMap)
    {
        pair.second = 0;
    }
    if (m_data)
    {
        FillEventClass(incl_empty, event_info, processName, particleMap, emptyMap);
    }
    else
    {
        FillEventClass(incl_empty, event_info, processName, particleMap, emptyMap, weight, systName);
    }
    // get the exclusive class
    if (!hasTooManyJets(countMap))
    {
        TEventClass *EventClasstoFill = exclEventClasses[excl_EC_name];
        if (EventClasstoFill == 0)
        {
            // EventClass not existing, create it
            EventClasstoFill = InitTEventClass(type, excl_EC_name, particleMap, countMap, absCharge, false);

            exclEventClasses[excl_EC_name] = EventClasstoFill;
        }
        if (m_data)
        {
            // data mode, so there are no weights at all
            FillEventClass(EventClasstoFill, event_info, processName, particleMap, countMap);
        }
        else
        {
            FillEventClass(EventClasstoFill, event_info, processName, particleMap, countMap, weight, systName);
        }
    }

    // fill inclusive Event Classes
    // create a list of particleNames in this countMap
    std::vector<std::string> particleNames;
    for (auto count : countMap)
    {
        particleNames.push_back(count.first);
    }
    // create empty referecne count map for recursion
    std::unordered_map<std::string, int> recCountMap = std::unordered_map<std::string, int>(countMap);
    for (auto &count : recCountMap)
    {
        count.second = 0;
    }
    std::string nameSuffixIncl = "+X";
    FillInclusiveRecursive(inclEventClasses,
                           particleNames,
                           type,
                           processName,
                           weight,
                           event_info,
                           0,
                           recCountMap,
                           countMap,
                           particleMap,
                           systName,
                           nameSuffixIncl);

    std::unordered_map<std::string, int> jetRecCountMap = std::unordered_map<std::string, int>(countMap);
    jetRecCountMap["Jet"] = 0;
    std::string nameSuffixJetIncl = "+NJets";
    FillInclusiveRecursive(jetInclEventClasses,
                           particleNames,
                           type,
                           processName,
                           weight,
                           event_info,
                           0,
                           jetRecCountMap,
                           countMap,
                           particleMap,
                           systName,
                           nameSuffixJetIncl);
}