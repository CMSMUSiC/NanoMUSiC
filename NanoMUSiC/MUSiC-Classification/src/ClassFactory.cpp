#include "ClassFactory.hpp"
#include "TError.h"
#include "TROOT.h"
#include "TStyle.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#pragma GCC diagnostic pop

using namespace std::string_literals;

ClassFactory::ClassFactory(const bool is_data,
                           const double cross_section,
                           const double filter_efficiency,
                           const double k_factor,
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

      m_data(is_data)
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

auto ClassFactory::analyse_event() -> void
{
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
    std::string const outFilePath("Analyzed.txt");
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

void ClassFactory::fillCutFlow(const bool pass_veto,
                               const bool pass_trigger_accept,
                               const bool pass_filter_accept,
                               const bool pass_generator_accept,
                               const bool pass_topo_accept,
                               const double weight)
{
    m_cutFlowUnweighted.Fill("all", 1.);
    m_cutFlowWeighted.Fill("all", weight);

    if (!pass_veto)
    {
        return;
    }
    m_cutFlowUnweighted.Fill("Veto", 1.);
    m_cutFlowWeighted.Fill("Veto", weight);

    if (!pass_trigger_accept)
    {
        return;
    }
    m_cutFlowUnweighted.Fill("trigger_accept", 1.);
    m_cutFlowWeighted.Fill("trigger_accept", weight);

    if (!pass_filter_accept)
    {
        return;
    }
    m_cutFlowUnweighted.Fill("filter_accept", 1.);
    m_cutFlowWeighted.Fill("filter_accept", weight);

    if (!pass_generator_accept)
    {
        return;
    }
    m_cutFlowUnweighted.Fill("generator_accept", 1.);
    m_cutFlowWeighted.Fill("generator_accept", weight);

    if (!pass_topo_accept)
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
                               const std::map<std::string, int> &countMap,
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

void ClassFactory::analyseEvent(const long Run,
                                const long LumiSection,
                                const long EventNum,
                                const std::string &Dataset,
                                const std::string &Filename,
                                const long EventNumPxlio,
                                const double genWeight,
                                const double PUWeight,
                                const double PUWeightUp,
                                const double PUWeightDown,
                                const std::string &scale_variation,
                                const int scale_variation_n, )
{

    // store current event data
    // this struct is defined in TEventClass.hpp
    EventInfo event_info;
    event_info.run = Run;
    event_info.lumisection = LumiSection;
    event_info.eventnum = EventNum;
    event_info.dataset = Dataset;
    event_info.filename = Filename;
    event_info.eventnumpxlio = EventNumPxlio;
    event_info.prefire_weight = 1.;
    event_info.prefire_weight_up = 1.;
    event_info.prefire_weight_down = 1.;
    if (!m_data)
    {
        pxl::EventView *GenEvtView = event->getObjectOwner().findObject<pxl::EventView>("Gen");
        event_info.central_weight = genWeight;
        event_info.pileup = PUWeight;
        event_info.pileup_up = PUWeightUp;
        event_info.pileup_down = PUWeightDown;

        event_info.process = m_lastprocess;
        event_info.has_scale_variation = false;
        if (scale_variation and int(scale_variation_n) != 0)
        {
            event_info.has_scale_variation = true;
            event_info.qcd_scale = scale_variation;
        }
    }
    else
    {
        event_info.central_weight = 1.;
        event_info.process = "";
        event_info.qcd_scale = "";
        event_info.pileup = 1.;
        event_info.pileup_up = 1.;
        event_info.pileup_down = 1.;
    }

    // by default, the event weight is 1
    double event_weight = 1;
    double pileup_weight = 1;

    std::string proc = m_lastprocess;
    prepareProcessName(proc);
    double total_event_weight = event_weight * pileup_weight * event_info.prefire_weight;
    if (!m_data)
    {
        CheckForCrossSection(proc);
        // we need to initialize some systematic info after the first event was loaded to determine, e.g.
        // the current process group
        if (!m_syst_initialized)
        {
            addConstantSystematics();
            m_syst_initialized = true;
        }
        pxl::EventView *GenEvtView = event->getObjectOwner().findObject<pxl::EventView>("Gen");

        m_pdfTool.setPDFWeights(GenEvtView);

        if (RecEvtView->hasUserRecord("prefiring_scale_factor"))
        {
            event_info.prefire_weight = RecEvtView->getUserRecord("prefiring_scale_factor");
            event_info.prefire_weight_up = RecEvtView->getUserRecord("prefiring_scale_factor_up");
            event_info.prefire_weight_down = RecEvtView->getUserRecord("prefiring_scale_factor_down");
        }

        // set the event weight to the actual value
        event_weight = GenEvtView->getUserRecord("genWeight");
        pileup_weight = GenEvtView->getUserRecord("PUWeight");
        total_event_weight = event_weight * pileup_weight * event_info.prefire_weight;

        // Fill differential systematics
        for (auto &systInfo : systematics.m_activeSystematics)
        {
            // Fill all shifted views for systematic
            for (auto &evtView : systInfo->eventViewPointers)
            {
                // fill this event
                Fill(evtView, total_event_weight, event_info, evtView->getName());
            }
        }
    }
    // count this event, no matter if accepted or not
    eventCounts[proc] += 1;
    totalEvents[proc] += (total_event_weight);
    totalEventsUnweighted[proc] += event_weight;

    bool filter_accept = RecEvtView->getUserRecord("filter_accept");
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
    Fill(RecEvtView, total_event_weight, event_info);
}
