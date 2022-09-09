#include "EventClassFactory/EventClassFactory.hh"
#include "TError.h"
#include "TROOT.h"
#include "TStyle.h"
#include "Tools/MConfig.hh"
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#pragma GCC diagnostic pop

#include "PDFInfo.hh"
#include "Tools/PXL/Sort.hh"

#include "EventClassFactory/Resolutions.hh"

using namespace std;
using namespace pxl;

#define TRIGGERSTREAM

void splitGammaRegions(std::map<std::string, std::vector<pxl::Particle *>> &map)
{
    return Splitting::split(
        map, [](pxl::Particle *particle) { return particle->getUserRecord("isEndcap"); }, "Gamma", "GammaEE",
        "GammaEB");
}

void splitEleRegions(std::map<std::string, std::vector<pxl::Particle *>> &map)
{
    return Splitting::split(
        map, [](pxl::Particle *particle) { return particle->getUserRecord("isEndcap"); }, "Ele", "EleEE", "EleEB");
}

EventClassFactory::EventClassFactory(const Tools::MConfig &cutconfig, const TOMLConfig &xsecs, EventSelector &selector,
                                     Systematics &syst_shifter, const std::string outfilename, const string hash,
                                     const bool debug)
    : m_XSections(xsecs), m_pdfTool(cutconfig, debug), m_selector(selector), systematics(syst_shifter),
      m_outfilename(outfilename), m_runHash(hash), m_debug(debug),

      m_cme(cutconfig.GetItem<double>("General.CME")),

      m_usePDF(cutconfig.GetItem<bool>("General.usePDF")), m_useTrigger(cutconfig.GetItem<bool>("General.UseTriggers")),
      m_use_prefire_check(cutconfig.GetItem<bool>("Prefire.check.use")),

      m_syst_initialized(false), m_cutFlowUnweighted(TH1F("cutflow_unweighted", "cutflow_unweighted", 10, 0, 10)),
      m_cutFlowWeighted(TH1F("cutflow_weighted", "cutflow_weighted", 10, 0, 10)),
      m_cutFlowFilterUnweighted(TH1F("cutflow_filter_unweighted", "cutflow_filter_unweighted", 10, 0, 10)),
      m_cutFlowFilterWeighted(TH1F("cutflow_filter_weighted", "cutflow_filter_weighted", 10, 0, 10)),
      m_lastprocessGroup("uninitialized"),

      m_muo_use(cutconfig.GetItem<bool>("Muon.use")), m_ele_use(cutconfig.GetItem<bool>("Ele.use")),
      m_tau_use(cutconfig.GetItem<bool>("Tau.use")), m_gam_use(cutconfig.GetItem<bool>("Gamma.use")),
      m_jet_use(cutconfig.GetItem<bool>("Jet.use")), m_met_use(cutconfig.GetItem<bool>("MET.use")),

      m_ec_recOnly(cutconfig.GetItem<bool>("EventClass.RecOnly")),
      m_ec_genOnly(cutconfig.GetItem<bool>("EventClass.GenOnly")),
      m_ec_charge_use(cutconfig.GetItem<bool>("EventClass.charge.use")),

      m_ec_sumpt_min(cutconfig.GetItem<double>("EventClass.SumPt.min")),
      m_ec_minv_min(cutconfig.GetItem<double>("EventClass.Minv.min")),
      m_ec_met_min(cutconfig.GetItem<double>("EventClass.MET.min")),

      m_num_listed_events_max(cutconfig.GetItem<int>("EventClass.num.ListedEvents.max")),
      m_max_jet_ec(cutconfig.GetItem<int>("EventClass.Jets.max")),

      m_events_by_sumpt(), m_events_by_minv(), m_events_by_met(),

      m_fudge_sumpt(cutconfig.GetItem<double>("Resolution.fudge.sumpt")),
      m_fudge_minv(cutconfig.GetItem<double>("Resolution.fudge.minv")),
      m_fudge_met(cutconfig.GetItem<double>("Resolution.fudge.met")),
      m_bin_size_min(cutconfig.GetItem<double>("Resolution.Bin.size.min")),

      m_useBJets(cutconfig.GetItem<bool>("Jet.BJets.use")),
      m_jet_bJets_algo(cutconfig.GetItem<std::string>("Jet.BJets.Algo")),
      m_gam_regions_use(cutconfig.GetItem<bool>("Gamma.regions.use")),
      m_ele_regions_use(cutconfig.GetItem<bool>("Ele.regions.use")), splittings(),

      m_fakeErrorMap(readFakeErrorMap(cutconfig)), m_chargeErrorMap(readChargeErrorMap(cutconfig)),

      m_xsOrderErrorMap(cutconfig), m_gen_rec_map(cutconfig)

{

    if (m_ec_genOnly and m_ec_recOnly)
    {
        stringstream error;
        error << "In config file: ";
        error << "'" << cutconfig.GetConfigFilePath() << "': ";
        error << "'EventClass.GenOnly' and 'EventClass.RecOnly' not allowed simultaneously!";
        throw Tools::config_error(error.str());
    }

    if (m_use_prefire_check)
    {
        std::cout << "Requesting simple prefire check for events" << std::endl;
    }

    m_data = cutconfig.GetItem<bool>("General.RunOnData");
    // suppress root Info messages:
    gErrorIgnoreLevel = 1001;
    gROOT->SetStyle("Plain");

    // create the splitting list for particle maps
    if (m_useBJets)
    {
        splittings.push_back(Splitting::splitBjets);
    }
    // for future test
    // if( m_useWjet ){
    //   list.push_back( Splitting::splitWfatjets );
    //}

    if (m_gam_regions_use)
    {
        splittings.push_back(splitGammaRegions);
    }

    if (m_ele_regions_use)
    {
        splittings.push_back(splitEleRegions);
    }

    m_cutFlowFilterWeighted.SetCanExtend(TH1::kAllAxes);

    m_cutFlowFilterUnweighted.SetCanExtend(TH1::kAllAxes);
}

// read map of fake errors from config file
std::map<std::string, double> EventClassFactory::readFakeErrorMap(const Tools::MConfig &cutconfig)
{
    std::map<std::string, double> fakeErrorMap;
    for (auto partName : Tools::getParticleTypeAbbreviations())
    {
        fakeErrorMap.emplace(partName, cutconfig.GetItem<double>(partName + ".Error.Fakes"));
    }
    return fakeErrorMap;
}

// read map of charge errors from config file
std::map<std::string, double> EventClassFactory::readChargeErrorMap(const Tools::MConfig &cutconfig)
{
    std::map<std::string, double> chargeErrorMap;
    std::vector<std::string> leptons = {"Muon", "Ele", "Tau"};
    for (auto partName : leptons)
    {
        chargeErrorMap.emplace(partName, cutconfig.GetItem<double>(partName + ".Error.Charge"));
    }
    return chargeErrorMap;
}

// read constant systematics and add them to the list of systematics
void EventClassFactory::addConstantSystematics()
{
    // map of maps [systName][particleType]->value
    //~ std::map<std::string, std::map< std::string, double > > systMaps;
    // should be read from configs in the future
    //~ systMaps.emplace("fake", m_fakeErrorMap );
    //~ systMaps.emplace("charge", m_chargeErrorMap );
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "fakeUp", "fakeUp", false));
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "fakeDown", "fakeDown", false));
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "alphasUp", "alphasUp", false));
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "alphasDown", "alphasDown", false));
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "prefireUp", "prefireUp", false));
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "prefireDown", "prefireDown", false));
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "pileupUp", "pileupUp", false));
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "pileupDown", "pileupDown", false));
    systematics.m_activeSystematics.push_back(
        new SystematicsInfo("General", getQCDSystWeightName("Up"), getQCDSystWeightName("Up"), false));
    systematics.m_activeSystematics.push_back(
        new SystematicsInfo("General", getQCDSystWeightName("Down"), getQCDSystWeightName("Down"), false));
    systematics.m_activeSystematics.push_back(
        new SystematicsInfo("General", "scale_factor_syst", "scale_factor_syst", false));
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "charge", "charge", false));
    systematics.m_activeSystematics.push_back(new SystematicsInfo("General", "luminosity", "luminosity", false));
    // loop over all maps for constant systematics
    //~ for( auto systMap : systMaps){
    //~ std::string systType = systMap.first;
    //~ for( auto systParticlePair : systMap.second ){
    //~ std::string partType = systParticlePair.first;
    //~ double fakeShift = systParticlePair.second;
    //~ std::string funcKey = partType + "_" + systType;
    //~ SystematicsInfo *thisSystematic =
    //~ new SystematicsInfo( partType, systType, funcKey, false, fakeShift );
    //~ systematics->m_activeSystematics.push_back( thisSystematic );
    //~ }
    //~ }
}
//----------------------------------------------------------------------

std::string EventClassFactory::getQCDSystWeightName(std::string shift_type) const
{
    return "qcdWeight" + m_lastprocessGroup + "_" + shift_type;
}

void EventClassFactory::FinishEventClasses(map<std::string, TEventClass *> &EvtClasses,
                                           std::set<std::string> &ProcessList, const string &Label)
{

    TFile OutputFile(Label.c_str(), "update");
    for (map<std::string, TEventClass *>::iterator Iter = EvtClasses.begin(); Iter != EvtClasses.end(); ++Iter)
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

//----------------------------------------------------------------------

void EventClassFactory::endJob(const Serializable *)
{

#ifdef TRIGGERSTREAM
    trig_stream.writeUniqueTriggers();
#endif

    // cutflows in separated root file
    TFile *fout_cutflow = new TFile("cutflow.root", "RECREATE");
    if (!m_data && !eventCounts.empty())
    {
        std::string proc = eventCounts.begin()->first;
        const double xsec = m_XSections.get<double>(proc + ".XSec");
        const double fEff = m_XSections.get<double>(proc + ".FilterEff");
        const double kFac = m_XSections.get<double>(proc + ".kFactor");
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
    for (map<std::string, unsigned int>::const_iterator proc_num = eventCounts.begin(); proc_num != eventCounts.end();
         ++proc_num)
    {
        ProcessList.insert(proc_num->first);
    }

    WriteAnalysisInfoToFile(ProcessList);

    string Label;

    if (m_outfilename.empty())
    {
        Label = "EC";
        for (std::set<std::string>::iterator Iter = ProcessList.begin(); Iter != ProcessList.end(); Iter++)
        {
            std::string process = (*Iter);
            cout << " Found Process " << process << endl;
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
    if (!ends_with(Label, ".root"))
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
    for (map<std::string, TEventClass *>::iterator Iter = _recEventClasses.begin(); Iter != _recEventClasses.end();
         Iter++)
    {
        if (Iter->second != 0)
        {
            delete Iter->second;
        }
    }
    for (map<std::string, TEventClass *>::iterator Iter = _recInclEventClasses.begin();
         Iter != _recInclEventClasses.end(); Iter++)
    {
        if (Iter->second != 0)
        {
            delete Iter->second;
        }
    }
    for (map<std::string, TEventClass *>::iterator Iter = _recJetInclEventClasses.begin();
         Iter != _recJetInclEventClasses.end(); Iter++)
    {
        if (Iter->second != 0)
        {
            delete Iter->second;
        }
    }
    if (!m_data)
    {
        for (map<std::string, TEventClass *>::iterator Iter = _genEventClasses.begin(); Iter != _genEventClasses.end();
             Iter++)
        {
            if (Iter->second != 0)
            {
                delete Iter->second;
            }
        }
        for (map<std::string, TEventClass *>::iterator Iter = _genInclEventClasses.begin();
             Iter != _genInclEventClasses.end(); Iter++)
        {
            if (Iter->second != 0)
            {
                delete Iter->second;
            }
        }
        for (map<std::string, TEventClass *>::iterator Iter = _genJetInclEventClasses.begin();
             Iter != _genJetInclEventClasses.end(); Iter++)
        {
            if (Iter->second != 0)
            {
                delete Iter->second;
            }
        }
    }
}

// Strip out extension sample strings from process name
void EventClassFactory::prepareProcessName(std::string &proc)
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

// This method searches for the entry for the given process in the XSec file and throws an exception if not found!
void EventClassFactory::CheckForCrossSection(string &process)
{

    std::string processKey = process + ".XSec";
    std::string processOrderKey = process + ".XSecOrder";
    std::string processGroupKey = process + ".ProcessGroup";

    // first check if the XSec for the given proces has already been found
    // (save the time searching the map again and again)
    if (m_lastprocess.compare(processKey) != 0)
    {
        if (!m_XSections.check_item(processKey))
        {
            throw std::runtime_error(
                "EventClassFactory::CheckForCrossSection( string process ): No value found for item: '" + processKey +
                "' in cross section file: '" + m_XSections.config_file_path() + "'. Please check it!");
        }
        if (!m_XSections.check_item(processKey))
        {
            throw std::runtime_error(
                "EventClassFactory::CheckForCrossSection( string process ): No XSecOrder found for item: '" +
                processKey + "' in cross section file: '" + m_XSections.config_file_path() + "'. Please check it!");
        }
        if (!m_XSections.check_item(processGroupKey))
        {
            throw std::runtime_error(
                "EventClassFactory::CheckForCrossSection( string process ): No processGroup found for item: '" +
                processKey + "' in cross section file: '" + m_XSections.config_file_path() + "'. Please check it!");
        }
        m_lastprocess = process;
        m_lastprocessOrder = m_XSections.get<std::string>(processOrderKey);
        if (m_lastprocessOrder == "")
        {
            cerr << "No process order set in scales.txt" << endl;
            exit(1);
        }
        std::string newprocessGroup = m_XSections.get<std::string>(processGroupKey);
        if (newprocessGroup == "")
        {
            newprocessGroup = m_lastprocess;
        }
        m_xsec = m_XSections.get<double>(process + ".XSec");
        m_fEff = m_XSections.get<double>(process + ".FilterEff");
        m_kFac = m_XSections.get<double>(process + ".kFactor");
        if (newprocessGroup != m_lastprocessGroup and newprocessGroup.find("uninitialized") != string::npos)
        {
            throw std::runtime_error(
                "EventClassFactory, trying to mix samples from different process groups in one run");
        }
        m_lastprocessGroup = newprocessGroup;
    }
}

void EventClassFactory::fillFilterCutFlow(const double weight)
{
    m_cutFlowFilterUnweighted.Fill("All", 1.);
    m_cutFlowFilterWeighted.Fill("All", weight);
    for (auto filter_pair : m_selector.getFilterMap())
    {
        if (filter_pair.second >= 0)
        {
            // clean label from possible prefixes
            std::string label = std::regex_replace(filter_pair.first, std::regex("AllFilters_p_"), "");
            label = std::regex_replace(label, std::regex("Flag_"), "");

            m_cutFlowFilterUnweighted.Fill(label.c_str(), 1.);
            m_cutFlowFilterWeighted.Fill(label.c_str(), weight);
        }
    }
}

void EventClassFactory::fillCutFlow(const pxl::Event *event, const double weight)
{
    pxl::EventView *recEvtView = event->getObjectOwner().findObject<pxl::EventView>("Rec");
    m_cutFlowUnweighted.Fill("all", 1.);
    m_cutFlowWeighted.Fill("all", weight);

    if (recEvtView->getUserRecord("Veto"))
        return;
    m_cutFlowUnweighted.Fill("Veto", 1.);
    m_cutFlowWeighted.Fill("Veto", weight);

    if (!recEvtView->getUserRecord("trigger_accept"))
        return;
    m_cutFlowUnweighted.Fill("trigger_accept", 1.);
    m_cutFlowWeighted.Fill("trigger_accept", weight);

    if (!recEvtView->getUserRecord("filter_accept"))
        return;
    m_cutFlowUnweighted.Fill("filter_accept", 1.);
    m_cutFlowWeighted.Fill("filter_accept", weight);

    if (!recEvtView->getUserRecord("generator_accept"))
        return;
    m_cutFlowUnweighted.Fill("generator_accept", 1.);
    m_cutFlowWeighted.Fill("generator_accept", weight);

    if (!recEvtView->getUserRecord("topo_accept"))
        return;
    m_cutFlowUnweighted.Fill("topo_accept", 1.);
    m_cutFlowWeighted.Fill("topo_accept", weight);
}

void EventClassFactory::fillCutFlow(const double sumPt, const double invMass, const double met, const int numPart,
                                    const std::map<std::string, int> &countMap, const double weight)
{
    if (sumPt < m_ec_sumpt_min)
        return;
    m_cutFlowUnweighted.Fill("sumPt_cut", 1.);
    m_cutFlowWeighted.Fill("sumPt_cut", weight);
    if (numPart > 1 and invMass < m_ec_minv_min)
        return;
    m_cutFlowUnweighted.Fill("invMass_cut", 1.);
    m_cutFlowWeighted.Fill("invMass_cut", weight);
    if (countMap.at("MET") > 0 and met < m_ec_met_min)
        return;
    m_cutFlowUnweighted.Fill("met_cut", 1.);
    m_cutFlowWeighted.Fill("met_cut", weight);
}

//----------------------------------------------------------------------

void EventClassFactory::analyseEvent(const pxl::Event *event)
{

    // store current event data
    // this struct is defined in TEventClass.hh
    EventInfo event_info;
    event_info.run = event->getUserRecord("Run");
    event_info.lumisection = event->getUserRecord("LumiSection");
    event_info.eventnum = event->getUserRecord("EventNum");
    event_info.dataset = event->getUserRecord("Dataset").asString();
    event_info.filename = event->getUserRecord("Filename").asString();
    event_info.eventnumpxlio = event->getUserRecord("EventNumPxlio");
    event_info.prefire_weight = 1.;
    event_info.prefire_weight_up = 1.;
    event_info.prefire_weight_down = 1.;
    if (!m_data)
    {
        pxl::EventView *GenEvtView = event->getObjectOwner().findObject<pxl::EventView>("Gen");
        event_info.central_weight = GenEvtView->getUserRecord("genWeight");
        event_info.pileup = GenEvtView->getUserRecord("PUWeight");
        event_info.pileup_up = GenEvtView->getUserRecord("PUWeightUp");
        event_info.pileup_down = GenEvtView->getUserRecord("PUWeightDown");

        event_info.process = GenEvtView->getUserRecord("Process").asString();
        event_info.has_scale_variation = false;
        if (GenEvtView->hasUserRecord("scale_variation") and int(GenEvtView->getUserRecord("scale_variation_n")) != 0)
        {
            event_info.has_scale_variation = true;
            event_info.qcd_scale = GenEvtView->getUserRecord("scale_variation").asString();
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

#ifdef TRIGGERSTREAM
    trig_stream.addEntry(event);
#endif

    // by default, the event weight is 1
    double event_weight = 1;
    double pileup_weight = 1;

    // access the original event data
    pxl::EventView *RecEvtView = event->getObjectOwner().findObject<pxl::EventView>("Rec");

    std::string proc = RecEvtView->getUserRecord("Process");
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

bool EventClassFactory::hasTooManyJets(std::map<std::string, int> countMap)
{
    unsigned int njets = countMap["Jet"] + countMap["bJet"];
    return njets > m_max_jet_ec;
}

//----------------------------------------------------------------------
void EventClassFactory::Fill(pxl::EventView *EvtView, double const weight, EventInfo &event_info, std::string systName)
{
    // MET filters
    if (!EvtView->getUserRecord("filter_accept"))
        return;
    // Cross cleaning of trigger streams
    if (EvtView->getUserRecord("Veto"))
        return;
    // Trigger string and offline cuts fullfiled for one trigger
    if (!EvtView->getUserRecord("trigger_accept"))
        return;
    // Generator cuts
    if (!EvtView->getUserRecord("generator_accept"))
        return;

    // simple prefiring check
    if (m_use_prefire_check && !EvtView->getUserRecord("pass_prefiring_check"))
        return;

    std::string type = EvtView->getUserRecord("Type");
    if ((type == "Gen" and m_ec_recOnly) or (type == "Rec" and m_ec_genOnly))
        return;

    // event with accepted topology?
    bool topo_accept = EvtView->getUserRecord("topo_accept");

    // method which creates and fills the EventClasses
    std::string processName = EvtView->getUserRecord("Process");

    // Delete 'ext' from process name ( via reference )
    prepareProcessName(processName);
    // EventClass specific

    std::map<std::string, int> countMap = std::map<std::string, int>();
    std::map<std::string, std::vector<pxl::Particle *>> particleMapTmp =
        std::map<std::string, std::vector<pxl::Particle *>>();
    if (topo_accept)
    {
        // Count this event only if it is accepted.
        if (systName.empty())
        {
            if (type == "Gen")
            {
                genTotalEventsWeighted[processName] += weight;
                genTotalEventsUnweighted[processName]++;
            }
            else
            {
                recTotalEventsWeighted[processName] += weight;
                recTotalEventsUnweighted[processName]++;
            }
        }

        // get a map containing only selected particles
        particleMapTmp = m_selector.getParticleLists(EvtView, false, splittings);
        particleMapTmp.erase("GammaEE");
    }
    ParticleMap particleMap(particleMapTmp);
    countMap = particleMap.getCountMap();

    // calculate the class name
    string excl_EC_name;
    string incl_empty_name;
    int absCharge = 0;
    if (m_ec_charge_use)
    {
        absCharge = abs(particleMap.getLeptonCharge(countMap));
    }
    excl_EC_name = TEventClass::calculateEventClass(countMap, std::map<std::string, int>(), orderParticleTypes);
    incl_empty_name = TEventClass::calculateEventClass(std::map<std::string, int>()) + "+X";

    // get the event class maps
    map<std::string, TEventClass *> &exclEventClasses = getReferenceExclusiveMap(type);
    map<std::string, TEventClass *> &inclEventClasses = getReferenceInclusiveMap(type);
    map<std::string, TEventClass *> &jetInclEventClasses = getReferenceJetInclusiveMap(type);

    // all accepted events are put in the Empty+X class
    TEventClass *incl_empty = inclEventClasses[incl_empty_name];
    if (incl_empty == 0)
    {
        // EventClass not existing, create it
        incl_empty = InitTEventClass(type, incl_empty_name, particleMap, std::map<std::string, int>(), 0.0, true);

        inclEventClasses[incl_empty_name] = incl_empty;
    }
    std::map<std::string, int> emptyMap = countMap;
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
    std::map<std::string, int> recCountMap = std::map<std::string, int>(countMap);
    for (auto &count : recCountMap)
    {
        count.second = 0;
    }
    std::string nameSuffixIncl = "+X";
    FillInclusiveRecursive(inclEventClasses, particleNames, type, processName, weight, event_info, 0, recCountMap,
                           countMap, particleMap, systName, nameSuffixIncl);

    std::map<std::string, int> jetRecCountMap = std::map<std::string, int>(countMap);
    jetRecCountMap["Jet"] = 0;
    std::string nameSuffixJetIncl = "+NJets";
    FillInclusiveRecursive(jetInclEventClasses, particleNames, type, processName, weight, event_info, 0, jetRecCountMap,
                           countMap, particleMap, systName, nameSuffixJetIncl);
}

void EventClassFactory::FillInclusiveRecursive(std::map<std::string, TEventClass *> &inclEventClasses,
                                               std::vector<std::string> partNameVector, std::string Type,
                                               std::string Process, double const weight, EventInfo &event_info,
                                               int iterator, std::map<std::string, int> recCountMap,
                                               std::map<std::string, int> refCountMap, ParticleMap &particleMap,
                                               std::string systName, std::string nameSuffix)
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
            if (m_ec_charge_use)
            {
                iCharge = abs(particleMap.getLeptonCharge(recCountMap));
            }
            EventClassName =
                TEventClass::calculateEventClass(recCountMap, std::map<std::string, int>(), orderParticleTypes) +
                nameSuffix;
            // Get the histo from the map per inclusive rec-Eventclass

            if (!hasTooManyJets(recCountMap))
            {
                TEventClass *InclEventClasstoFill = inclEventClasses[EventClassName];
                if (InclEventClasstoFill == 0 && !hasTooManyJets(recCountMap))
                {
                    InclEventClasstoFill = InitTEventClass(Type, EventClassName, particleMap, recCountMap,
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
                    FillEventClass(InclEventClasstoFill, event_info, Process, particleMap, recCountMap, weight,
                                   systName);
                }
            }
        }
        else
        { // we are not at the deepest level yet
            FillInclusiveRecursive(inclEventClasses, partNameVector, Type, Process, weight, event_info, iterator + 1,
                                   recCountMap, refCountMap, particleMap, systName, nameSuffix);
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
TEventClass *EventClassFactory::InitTEventClass(std::string const &Type, std::string const &ECName,
                                                ParticleMap &particleMap, std::map<std::string, int> countMap,
                                                int const absCharge, bool const inclusiveEC) const
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
    for (auto &systInfo : systematics.m_activeSystematics)
    {
        if (!systInfo->m_isDifferential)
            systNames.emplace(systInfo->m_funcKey);
        else
        {
            for (auto &evtView : systInfo->eventViewPointers)
            {
                systNames.emplace(evtView->getName());
            }
        }
    }
    std::cout << "Adding new class " << ECName << std::endl;
    return new TEventClass(Type, ECName, m_runHash, m_data, m_cme, countMap, m_useBJets, distTypeBins, m_ec_charge_use,
                           absCharge, inclusiveEC, distTypMins, m_pdfTool.m_pdfInfo.getNumPDFs(), distTypeMinsRequire,
                           m_XSections.get<double>("Lumi"), systNames);
}

void EventClassFactory::FillEventClass(TEventClass *EventClassToFill, EventInfo &event_info, std::string const &process,
                                       ParticleMap &particleMap, std::map<std::string, int> &countMap,
                                       double const eventWeight, std::string systName)
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
    pxl::Particle vec_sum = pxl::Particle();

    // create empty maps
    // for pxl particle vectors with reduced number of parts for inclusive classes
    std::map<std::string, std::vector<pxl::Particle *>> realParticleMap;
    // for fake counting
    std::map<std::string, int> fakeMap;
    std::map<std::string, int> chargeFakeMap;

    // Fill all considered particle types to the event distributions (distTypes)
    sumPt = particleMap.getSumPt(countMap);
    MET = particleMap.getMET(countMap);
    Minv = particleMap.getMass(countMap);
    if (ECtype == "Rec" and not minimalFill)
    {
        fakeMap = particleMap.getFakeMap(countMap);
        chargeFakeMap = particleMap.getChargeFakeMap(countMap);
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
    if (m_ec_charge_use and not minimalFill)
    {
        for (auto errorPair : m_chargeErrorMap)
        {
            weight_charge_error += std::pow(errorPair.second * chargeFakeMap[errorPair.first], 2);
        }
        weight_charge_error = sqrt(weight_fake_error);
    }

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
    { // Fill differential systematic
        // the eventclass may not be filled yet.make sure the xs uncert is added
        // even though syst weight are not filled for differential systematics
        std::string xsUncertName = "xs" + m_lastprocessGroup + m_lastprocessOrder;
        if (!EventClassToFill->hasSystematic(xsUncertName))
        {
            EventClassToFill->addSystematic(xsUncertName);
        }
        EventClassToFill->FillDifferentialSystematic(process, values, weight, m_pdfTool.getPDFWeights(), systName);
    }
    else
    {
        // Fill standard MC and constant systematics
        std::map<std::string, double> systWeights;
        // fake
        systWeights["fakeUp"] = 1. + weight_fake_error;
        systWeights["fakeDown"] = 1. - weight_fake_error;
        systWeights["charge"] = 1 + weight_charge_error;
        systWeights["luminosity"] = 1 + m_XSections.get<double>("Global.ScalefactorError");
        systWeights["scale_factor_syst"] = 1 + scale_factor_syst_error;
        std::pair<double, double> asUncerts = m_pdfTool.getAsWeights();
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
        EventClassToFill->Fill(process, values, resolutionMap, weight, systWeights, m_pdfTool.getPDFWeights());
    }
}

bool EventClassFactory::CheckEventToList(double const sumPt, double const Minv, double const MET) const
{
    // Simply accept all events with anything in them, atm.
    if (sumPt > 0.0 or Minv > 0.0 or MET > 0.0)
        return true;
    return false;
}

void EventClassFactory::FillEventList(std::string const &ECname, EventInfo const &event_info)
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

std::vector<EventClassFactory::EventInfo> EventClassFactory::GetListedEvents(std::string const &ECname)
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

void EventClassFactory::WriteEventListToFile(std::string const &ECname)
{
    std::vector<EventInfo> const event_list = GetListedEvents(ECname);

    std::ofstream outfile(("Event-lists/" + ECname + ".txt").c_str());
    // 'event_list' is always empty for 'Rec_Empty'.
    if (event_list.size() > 0)
    {
        outfile << "# dataset = " << event_list.at(0).dataset << std::endl;
    }
    outfile << std::string(160, '#') << endl;
    outfile << "#" << std::setw(12) << "sumpt:" << std::setw(13) << "minv:" << std::setw(11) << "met:" << std::setw(11)
            << "run:" << std::setw(15) << "lumi section:" << std::setw(16) << "event number:" << std::setw(72)
            << "pxlio file:" << std::setw(9) << "num:" << std::endl;
    outfile << std::string(160, '#') << endl;

    std::vector<EventInfo>::const_iterator event_it;
    int sumptMapSize = m_events_by_sumpt[ECname].size();
    int minvMapSize = m_events_by_minv[ECname].size();
    int counter = 0;
    outfile << "# Begin SumPt list" << endl;
    for (event_it = event_list.begin(); event_it != event_list.end(); ++event_it)
    {
        if ((float)counter / sumptMapSize == 1.)
            outfile << "# Begin Minv list" << endl;
        if ((float)counter / (minvMapSize + sumptMapSize) == 1.)
            outfile << "# Begin MET list" << endl;
        outfile << std::setw(13) << event_it->sumpt << "  " << std::setw(11) << event_it->minv << "  " << std::setw(9)
                << event_it->met << "  " << std::setw(9) << event_it->run << "  " << std::setw(13)
                << event_it->lumisection << "  " << std::setw(14) << event_it->eventnum << "  " << std::setw(70)
                << event_it->filename << "  " << std::setw(7) << event_it->eventnumpxlio << std::endl;
        counter++;
        ;
    }
    outfile.close();
}

void EventClassFactory::WriteAnalysisInfoToFile(std::set<std::string> const &processList)
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
