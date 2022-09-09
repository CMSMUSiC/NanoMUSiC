#include "TEventClass.hh"
#include "TCanvas.h"
#include "THStack.h"
#include "TROOT.h"
#include "TStyle.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Tools.hh"

using namespace std;

ClassImp(TEventClass)

    //-------------Konstructor-------------------------------------------------------------

    TEventClass::TEventClass(const std::string &EventType, const std::string &EventClassType, const std::string runhash,
                             bool const data, double const cme, std::map<std::string, int> countmap,
                             const bool analyzedBjets, std::map<std::string, std::vector<double>> distTypeBins,
                             const bool analyzedCharge, const int numCharge, const bool isInclusive,
                             std::map<std::string, double> distTypMins, unsigned int m_numPDFvariations,
                             std::map<std::string, double> distTypeMinsRequire, const double lumi,
                             std::set<std::string> systNames, std::map<std::string, std::string> ECItemShortlist,
                             double const bin_size_min)
    : TNamed(EventType + EventClassType, EventType + EventClassType), m_eventClassType(EventClassType),
      m_eventType(EventType), m_runHash(runhash), m_isInclusive(isInclusive), m_data(data), m_cme(cme), m_lumi(lumi),
      m_countmap(countmap), m_distTypeMins(distTypMins), m_distTypeMinsRequire(distTypeMinsRequire),
      m_analyzedCharge(analyzedCharge), m_numCharge(numCharge), m_analyzedBjets(analyzedBjets),
      m_distTypeBinInfo(distTypeBins), m_systNames(systNames), m_numPDFvariations(m_numPDFvariations),
      m_bin_size_min(bin_size_min), m_distTypeBinsHisto({}),
      m_resolutionsMap(std::map<std::string, std::map<std::string, TH2F *>>()),
      processGroupMap(std::map<std::string, std::string>()), m_ECItemShortlist(ECItemShortlist), m_isEmpty(false),
      m_is_filled(false), m_scaledToXSec(false)
{
    // this is hardcoded for now but only at this point
    m_histnamePDFup = "pdfuponly";
    // pdfs are a special case and not entered rom outside
    m_systNames.insert(m_histnamePDFup);
    // create dummy histos for each distribution
    for (auto &binInfo : m_distTypeBinInfo)
    {
        std::string histname = "dummy_" + binInfo.first;
        TH1F *temp_Histo =
            new TH1F(histname.c_str(), histname.c_str(), (binInfo.second).size() - 1, &(binInfo.second)[0]);
        m_distTypeBinsHisto[binInfo.first] = temp_Histo;
    }
}

//-------------Destructor-------------------------------------------------------------

TEventClass::~TEventClass()
{
    // Delete Histos & other stuff
    // Loop over all histos in the map and delete them

    for (auto &procIter : allProcHistoMap)
    {
        for (auto &distTypeMap : procIter.second)
        {
            if (distTypeMap.second != 0)
                delete distTypeMap.second;
        }
    }

    for (auto &procIter : allProcHistoMapUnweighted)
    {
        for (auto &distTypeMap : procIter.second)
        {
            if (distTypeMap.second != 0)
                delete distTypeMap.second;
        }
    }

    for (auto &procIter : SystematicsProcHistoMap)
    {
        for (auto &distTypeMap : procIter.second)
        {
            for (auto &histpair : distTypeMap.second)
            {
                if (histpair.second != 0)
                    delete histpair.second;
            }
        }
    }
    dropPDFHistograms();

    for (auto &procIter : m_resolutionsMap)
    {
        for (auto &histpair : procIter.second)
        {
            if (histpair.second != 0)
                delete histpair.second;
        }
    }
}

//-------------Copyconstructor-------------------------------------------------------------

// ECMerger2 needs to create empty classes with basically all information but
// the actual histogram entries.

TEventClass::TEventClass(const TEventClass &orig, bool empty) : TNamed(orig.GetName(), orig.GetTitle())
{
    copyValues(orig, empty);
}

TEventClass::TEventClass(const TEventClass &orig) : TNamed(orig.GetName(), orig.GetTitle())
{
    copyValues(orig, orig.isEmpty());
}

//-------------operator=-------------------------------------------------------------

TEventClass &TEventClass::operator=(const TEventClass &rhs)
{
    if (this != &rhs)
    {
        TNamed(rhs.GetName(), rhs.GetTitle());
        copyValues(rhs, rhs.isEmpty());
    }
    return *this;
}

void TEventClass::copyValues(const TEventClass &rhs, bool empty)
{
    m_eventClassType = rhs.m_eventClassType;
    m_eventType = rhs.m_eventType;
    m_runHash = rhs.m_runHash;
    m_isInclusive = rhs.m_isInclusive;
    m_data = rhs.m_data;
    m_cme = rhs.m_cme;
    m_lumi = rhs.m_lumi;

    m_changelog = rhs.m_changelog;

    m_countmap = rhs.m_countmap;

    m_distTypeMins = rhs.m_distTypeMins;
    m_distTypeMinsRequire = rhs.m_distTypeMinsRequire;

    m_analyzedCharge = rhs.m_analyzedCharge;
    m_numCharge = rhs.m_numCharge;

    m_analyzedBjets = rhs.m_analyzedBjets;

    m_distTypeBinInfo = rhs.m_distTypeBinInfo;
    m_systNames = rhs.m_systNames;
    m_numPDFvariations = rhs.m_numPDFvariations;
    m_histnamePDFup = rhs.m_histnamePDFup;
    m_bin_size_min = rhs.m_bin_size_min;

    m_distTypeBinsHisto = rhs.m_distTypeBinsHisto;

    eventCount = rhs.eventCount;
    totalEvents = rhs.totalEvents;
    totalEventsUnweighted = rhs.totalEventsUnweighted;

    allProcHistoMap = std::map<std::string, std::map<std::string, TH1F *>>();
    allProcHistoMapUnweighted = std::map<std::string, std::map<std::string, TH1F *>>();
    SystematicsProcHistoMap = std::map<std::string, std::map<std::string, std::map<std::string, TH1F *>>>();
    PDFvariedallProcHistoMap = std::map<std::string, std::map<std::string, std::map<std::string, TH1F *>>>();
    m_resolutionsMap = std::map<std::string, std::map<std::string, TH2F *>>();
    processGroupMap = rhs.processGroupMap;
    m_ECItemShortlist = rhs.m_ECItemShortlist;
    m_globalProcessList = set<string>(); // In case this is *not* meant to be an empty EventClass, the variable is taken
                                         // from rhs. See below.

    m_scaleFactor = rhs.m_scaleFactor;
    m_crossSection = rhs.m_crossSection;

    m_isEmpty = empty;
    m_is_filled = rhs.m_is_filled;
    m_scaledToXSec = rhs.m_scaledToXSec;
    scanResults_nData = rhs.scanResults_nData;
    scanResults_pseudo_nData = rhs.scanResults_pseudo_nData;
    scanResults_nMC = rhs.scanResults_nMC;
    scanResults_pseudo_nMC = rhs.scanResults_pseudo_nMC;
    scanResults_totalUncert = rhs.scanResults_totalUncert;
    scanResults_pseudo_totalUncert = rhs.scanResults_pseudo_totalUncert;
    scanResults_lowerEdges = rhs.scanResults_lowerEdges;
    scanResults_pseudo_lowerEdges = rhs.scanResults_pseudo_lowerEdges;
    scanResults_Widths = rhs.scanResults_Widths;
    scanResults_pseudo_Widths = rhs.scanResults_pseudo_Widths;
    scanResults_CompareScores = rhs.scanResults_CompareScores;
    scanResults_pseudo_CompareScores = rhs.scanResults_pseudo_CompareScores;
    scanResults_dicedData = rhs.scanResults_dicedData;

    // The following is only called if you need a "real" copy constructor.
    if (not m_isEmpty)
    {
        m_globalProcessList = rhs.m_globalProcessList;
        // info structure for nested maps allProcHistoMap[process][distribution]
        for (auto &procIter : rhs.allProcHistoMap)
        {
            for (auto &distmap : procIter.second)
            {
                TH1F *histcopy = new TH1F(*distmap.second);
                allProcHistoMap[procIter.first][distmap.first] = histcopy;
            }
        }
        for (auto &procIter : rhs.allProcHistoMapUnweighted)
        {
            for (auto &distmap : procIter.second)
            {
                TH1F *histcopy = new TH1F(*distmap.second);
                allProcHistoMapUnweighted[procIter.first][distmap.first] = histcopy;
            }
        }
        //  PDFs and Systematics
        for (auto &procIter : rhs.PDFvariedallProcHistoMap)
        {
            for (auto &distmap : procIter.second)
            {
                for (auto &histpair : distmap.second)
                {
                    TH1F *histcopy = new TH1F(*histpair.second);
                    PDFvariedallProcHistoMap[procIter.first][distmap.first][histpair.first] = histcopy;
                }
            }
        }

        for (auto &procIter : rhs.SystematicsProcHistoMap)
        {
            for (auto &distmap : procIter.second)
            {
                for (auto &histpair : distmap.second)
                {
                    TH1F *histcopy = new TH1F(*histpair.second);
                    SystematicsProcHistoMap[procIter.first][distmap.first][histpair.first] = histcopy;
                }
            }
        }

        // Resolutions
        for (auto &procIter : rhs.m_resolutionsMap)
        {
            for (auto distmap : procIter.second)
            {
                TH2F *histcopy = new TH2F(*distmap.second);
                m_resolutionsMap[procIter.first][distmap.first] = histcopy;
            }
        }
    }
}

// Initialise histogram vector with all the needed distributions.
// To make it more convenient, there is an enum defined in the .hh file.
void TEventClass::InitializeHistos(std::string const &process)
{
    std::map<std::string, TH1F *> &histos = allProcHistoMap[process];
    std::map<std::string, TH1F *> &histosUnweighted = allProcHistoMapUnweighted[process];
    std::map<std::string, std::map<std::string, TH1F *>> &systHistos = SystematicsProcHistoMap[process];

    string histoNameBase = process + m_eventType + m_eventClassType;
    string histoName;
    //~ std::cout << "init hists" << std::endl;
    // Loop over all base distributions
    for (std::pair<std::string, std::vector<double>> distmap : m_distTypeBinInfo)
    {
        std::string distType = distmap.first;
        histoName = histoNameBase + "_" + distType;
        TH1F *temp_Histo =
            new TH1F(histoName.c_str(), histoName.c_str(), (distmap.second).size() - 1, &(distmap.second)[0]);
        temp_Histo->Sumw2();
        histos[distType] = temp_Histo;
        std::string histoNameUnweight = histoName + "Unweighted";
        temp_Histo = new TH1F(histoNameUnweight.c_str(), histoNameUnweight.c_str(), (distmap.second).size() - 1,
                              &(distmap.second)[0]);
        temp_Histo->Sumw2();
        histosUnweighted[distType] = temp_Histo;
        // Systematics are only computed for MC.
        if (not m_data)
        {
            // add hists for systematics
            for (std::string systName : m_systNames)
            {
                std::string systHistoName = histoName + "_" + systName;
                // std::cout << "systHistoName " <<  systHistoName <<std::endl;
                systHistos[distType].insert(std::pair<std::string, TH1F *>(
                    systName, new TH1F(systHistoName.c_str(), systHistoName.c_str(), (distmap.second).size() - 1,
                                       &(distmap.second)[0])));
            }
        }
        //~ std::cout << "init hists end" << std::endl;
    }
}

void TEventClass::InitializePDFHistos(const string &process, const vector<float> &PDFweights)
{
    std::map<std::string, std::map<std::string, TH1F *>> &PDFhistos = PDFvariedallProcHistoMap[process];

    string histoNameBase = process + m_eventType + m_eventClassType;
    string histoName;
    // weight counter
    int i = 0;
    for (vector<float>::const_iterator PDFweight = PDFweights.begin(); PDFweight != PDFweights.end(); ++PDFweight)
    {
        i++;
        for (auto &distmap : m_distTypeBinInfo)
        {
            std::string distType = distmap.first;
            histoName = histoNameBase + "_" + distType + "_PDFvariation_" + std::to_string(i);
            //~ std::cout << histoName << std::endl;
            PDFhistos[distType].insert(std::pair<std::string, TH1F *>(
                std::to_string(i),
                new TH1F(histoName.c_str(), histoName.c_str(), (distmap.second).size() - 1, &(distmap.second)[0])));
        }
    }
}

void TEventClass::InitializeResolutions(std::string const &process)
{
    // Get a reference on the vector, so it is actually changed!
    std::map<std::string, TH2F *> &resolutions = m_resolutionsMap[process];

    std::string const histoNameBase = process + m_eventType + m_eventClassType;

    // One 10th of the center of mass energy should be enough for
    // resolution plots.
    double const Emax = m_cme / 10;

    // Bin size should not be smaller than the smallest bin used for the binning.
    unsigned int const Nbins = Emax / m_bin_size_min;

    // use bin info to determine which dist Types exist
    for (auto binInfo : m_distTypeBinInfo)
    {
        std::string distType = binInfo.first;
        std::string histoName = histoNameBase + "_" + distType + "_Resolution";
        resolutions.emplace(distType,
                            new TH2F(TString(histoName), ("Distribution of the resolution of " + distType).c_str(),
                                     Nbins, 0, Emax, Nbins, 0, Emax));
    }
}

void TEventClass::Fill(std::string const &process,
                       std::map<std::string, double> values, // values for each distType that should be filled
                       std::map<std::string, std::pair<double, double>> resolution_value, double const weight,
                       std::map<std::string, double> systWeights, std::vector<float> const &PDFweights)
{

    std::string histoNameBase = process + m_eventType + m_eventClassType;
    std::string histoName;
    std::string histoNamePDF;
    // Get histograms for 'process', check if they are already initialised.
    std::map<std::string, TH1F *> &histos = allProcHistoMap[process];
    std::map<std::string, TH1F *> &histosUnweighted = allProcHistoMapUnweighted[process];
    if (histos.size() == 0)
        InitializeHistos(process);
    // Get PDF variations
    std::map<std::string, std::map<std::string, TH1F *>> &PDFhistos = PDFvariedallProcHistoMap[process];
    auto &systhistos = SystematicsProcHistoMap[process];
    if (PDFhistos.size() == 0)
        InitializePDFHistos(process, PDFweights);
    double_t usedWeight = weight;
    if (m_data)
        usedWeight = 1.;
    for (auto valuepair : values)
    {
        std::string distType = valuepair.first;
        histoName = histoNameBase + "_" + distType;
        // Fill standard distributions
        histos[distType]->Fill(valuepair.second, usedWeight);
        // Fill standard distribution unweighted
        histosUnweighted[distType]->Fill(valuepair.second);
        if (!m_data)
        { // Fill constant systematics with custom usedWeights
            for (auto systWeight : systWeights)
            {
                std::string systHistoName = systWeight.first;
                auto systHisto = systhistos[distType].find(systHistoName);
                if (systHisto != systhistos[distType].end())
                {
                    systhistos[distType][systWeight.first]->Fill(valuepair.second, usedWeight * systWeight.second);
                }
                else
                {
                    stringstream errorMessage;
                    errorMessage << "In EventClass '" << GetName() << "' ";
                    errorMessage << "in process '" << process << "' ";
                    errorMessage << "No syst histo " << systHistoName;
                    errorMessage << endl;
                    throw runtime_error(errorMessage.str());
                }
            }
            // Initialize PDF histograms only for MC and if not JES up/down.
            int i = 0;
            for (float PDFweight : PDFweights)
            {
                i++;
                std::string key = std::to_string(i);
                if (PDFhistos[distType].find(key) == PDFhistos[distType].end())
                {
                    stringstream errorMessage;
                    errorMessage << "In EventClass '" << GetName() << "' ";
                    errorMessage << "in process '" << process << "' ";
                    errorMessage << "No PDF weight histo " << key;
                    errorMessage << endl;
                    throw runtime_error(errorMessage.str());
                }
                PDFhistos[distType][key]->Fill(valuepair.second, PDFweight * usedWeight);
            }
        }
    }
    // Get resolution histograms for 'process', check if they are already initialised.
    auto &resolutionHistos = m_resolutionsMap[process];
    if (resolutionHistos.size() == 0)
        InitializeResolutions(process);

    if (m_data)
    {
        usedWeight = 1.;
    }
    else
    {
        usedWeight = weight;
    }
    for (auto resType : resolution_value)
    {
        std::pair<double, double> valuePair = resType.second;
        std::string distType = resType.first;
        resolutionHistos[distType]->Fill(valuePair.first, valuePair.second, usedWeight);
    }

    if (!m_is_filled)
        m_is_filled = true;
}
// Fill for differential systematics
void TEventClass::FillDifferentialSystematic(std::string const &process, std::map<std::string, double> values,
                                             double const weight, std::vector<float> const &PDFweights,
                                             std::string systName)
{
    //~ std::cout << "in fill" <<std::endl;
    //~ auto& systhistos = SystematicsProcHistoMap.find( process );
    // Due to migration it is possible that a differential syst is the first
    // action that tries to Fill a eventClass
    if (SystematicsProcHistoMap.find(process) == SystematicsProcHistoMap.end())
    {
        InitializeHistos(process);
        InitializePDFHistos(process, PDFweights);
    }

    auto &resolutionHistos = m_resolutionsMap[process];
    if (resolutionHistos.size() == 0)
        InitializeResolutions(process);

    for (auto valuepair : values)
    {
        if (SystematicsProcHistoMap[process][valuepair.first].find(systName) ==
            SystematicsProcHistoMap[process][valuepair.first].end())
        {
            stringstream errorMessage;
            errorMessage << "In Class '" << m_eventClassType << "' ";
            errorMessage << "In Process '" << process << "' ";
            errorMessage << "No systematic '" << systName << "' ";
            errorMessage << endl;
            throw runtime_error(errorMessage.str());
        }
        SystematicsProcHistoMap[process][valuepair.first][systName]->Fill(valuepair.second, weight);
    }
    if (!m_is_filled)
        m_is_filled = true;
}
//---------------Return Process list of specific EventClass -------------------------------------

std::set<std::string> TEventClass::ProcessList()
{
    // get the keys from the _SumPtHistos maps
    std::set<std::string> procList;
    for (auto &procIter : allProcHistoMap)
    {
        procList.insert(procIter.first);
    }
    return procList;
}

// return the number of analyzed events for the requested process, without any weighting
unsigned int TEventClass::getEventCount(const std::string &process) const
{
    if (not process.empty())
    {
        if (eventCount.find(process) != eventCount.end())
        {
            return eventCount.at(process);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        unsigned int sum = 0;
        for (auto histpair : eventCount)
        {
            sum += histpair.second;
        }
        return sum;
    }
}

// return the sum of weight for all analyzed events for the requested process, weighted to lumi and cross section
double TEventClass::getTotalEvents(const std::string &process) const
{
    if (not process.empty())
    {
        if (totalEvents.find(process) != totalEvents.end())
        {
            return totalEvents.at(process);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        double sum = 0;
        for (auto histpair : totalEvents)
        {
            sum += histpair.second;
        }
        return sum;
    }
}

// return the sum of weight for all analyzed events for the requested process, not weighted
double TEventClass::getTotalEventsUnweighted(const std::string &process) const
{
    if (not process.empty())
    {
        if (totalEventsUnweighted.find(process) != totalEventsUnweighted.end())
        {
            return totalEventsUnweighted.at(process);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        double sum = 0;
        for (auto histpair : totalEventsUnweighted)
        {
            sum += histpair.second;
        }
        return sum;
    }
}

std::map<std::string, std::map<std::string, TH1F *>> TEventClass::getPDFWeightsMap(const std::string &process)
{
    auto iter = PDFvariedallProcHistoMap.find(process);
    // check existence of process
    if (iter != PDFvariedallProcHistoMap.end())
    {
        return iter->second;
    }
    return std::map<std::string, std::map<std::string, TH1F *>>();
}

//---------------Set the PDF histos for a certain process (needed for merging)------------

void TEventClass::setPDFWeights(const string &process,
                                const std::map<std::string, std::map<std::string, TH1F *>> &pdfDistMap)
{
    std::map<std::string, TH1F *> newHistos = {};
    for (auto &pdfIter : pdfDistMap)
    {
        std::string distType = pdfIter.first;
        for (auto &histIter : pdfIter.second)
        {
            std::string histName = histIter.first;
            newHistos.emplace(std::make_pair(histName, (TH1F *)histIter.second->Clone(histName.c_str())));
        }
        PDFvariedallProcHistoMap[process][distType] = newHistos;
        newHistos.clear();
    }
}

//---------------Scale Histos to Lumi -----------------------------------------------------------

void TEventClass::scaleAllHistograms(double relativeFactor)
{
    for (const std::string &process : m_globalProcessList)
    {
        // Absolute scaling does not make sense for all the various processes,
        // so we assume relative scaling.
        scaleAllHistogramsForProcess(process, relativeFactor, /*is_absolute=*/false);
    }
}

void TEventClass::scaleAllHistogramsForProcess(std::string process, double factor, bool is_absolute)
{

    // If scale factor has not been loaded into map (yet), calculate it from
    // total event numbers.
    if (m_scaleFactor.find(process) == m_scaleFactor.end())
    {
        m_scaleFactor[process] = totalEvents[process] / totalEventsUnweighted[process];
    }

    const double current_scale_factor = m_scaleFactor[process];

    // cross check: compare to totalEvents / totalEventsUnweighted
    const double ref = totalEvents[process] / totalEventsUnweighted[process];
    const double relerr = std::abs(current_scale_factor - ref) / ref;
    assert(relerr < 1e-5);

    double relative_scale_factor, destination_scale_factor;

    // Formula: dst = rel*curr => rel = dst/curr;
    if (is_absolute)
    {
        // Absolute scaling: set dst, calculate rel
        destination_scale_factor = factor;
        relative_scale_factor = destination_scale_factor / current_scale_factor;
    }
    else
    {
        // Relative scaling: set rel, calculate abs
        relative_scale_factor = factor;
        destination_scale_factor = relative_scale_factor * current_scale_factor;
    }

    if (relative_scale_factor != 1.0)
    {
        // scale all known histograms belonging to this process
        scaleHistoMap(allProcHistoMap[process], relative_scale_factor);
        scaleHistoMap(allProcHistoMapUnweighted[process], relative_scale_factor);
        scaleHistoMap(PDFvariedallProcHistoMap[process], relative_scale_factor);
        scaleHistoMap(SystematicsProcHistoMap[process], relative_scale_factor);
        scaleHistoMap(m_resolutionsMap[process], relative_scale_factor);

        totalEvents[process] *= relative_scale_factor;
    }

    m_scaleFactor[process] = destination_scale_factor;
}

template <typename T> void TEventClass::scaleHistoMap(std::map<std::string, T> &map, double relativeFactor)
{
    for (std::pair<const std::string, T> &entry : map)
    {
        scaleHistoMap(entry.second, relativeFactor);
    }
}

void TEventClass::scaleHistoMap(TH1 *histogram, double relativeFactor)
{
    histogram->Scale(relativeFactor);
}

void TEventClass::setLumi(double targetLumi)
{
    if (not m_data)
    {
        throw runtime_error("Setting lumi without scaling for MC. Use scaleLumi( double ) instead?");
    }

    m_lumi = targetLumi;
}

void TEventClass::scaleLumi(double targetLumi)
{
    // Scales histograms to target luminosity, should only be called for MC.

    if (m_data)
    {
        throw runtime_error("Scaling lumi to crosssection. Use setLumi( double ) instead?");
    }

    const double relative_scale_factor = targetLumi / m_lumi;
    scaleAllHistograms(relative_scale_factor);
    m_lumi = targetLumi;
}

void TEventClass::changeCrossSection(std::string process, double cross_section)
{
    // Change cross section of selected process
    // Rescales (just like lumi), is still experimental!

    if (m_data)
    {
        throw runtime_error("Changing cross section for data does not make sense.");
    }

    const double relative_scale_factor = cross_section / m_crossSection.at(process);
    scaleAllHistogramsForProcess(process, relative_scale_factor, /*is_absolute=*/false);
    m_crossSection[process] = cross_section;
}

void TEventClass::scaleAllCrossSections(double factor)
{
    if (m_data)
    {
        throw runtime_error("Changing cross section for data does not make sense.");
    }

    for (const std::string &process : m_globalProcessList)
    {
        double new_cross_section = m_crossSection.at(process) * factor;
        changeCrossSection(process, new_cross_section);
    }
}

void TEventClass::scale(const double totalXsec)
{
    // Loops over all histograms and scales them according to their cross section.
    // Should not be called twice (and will throw error if done so).

    if (m_scaledToXSec)
    {
        // In general, the code below should be able to handle this quite well,
        // but this hints to other problems...
        throw runtime_error("Scaling to crosssection has been requested twice.");
    }

    // change the scale factor/cross sections for the whole global process list
    for (const std::string &process : m_globalProcessList)
    {

        // Save the total cross section for reference (used as safety-feature
        // when merging event classes).
        m_crossSection[process] = totalXsec;

        // Calculate and store the scale factor

        // unweighted events is the sum of generator weights for *all* produced
        // events (including skipped events)
        double const unweighted_events = totalEventsUnweighted[process];

        double const scale_factor = totalXsec * m_lumi / unweighted_events;

        // apply the scale factor to all histograms of this process
        // absolute scaling takes into account current scale factor
        scaleAllHistogramsForProcess(process, scale_factor, /*is_absolute=*/true);
    }

    // Set flag that we have already performed scaling, will throw error on re-scaling
    m_scaledToXSec = true;
}

// Get pointer to hist
TH1F *TEventClass::getHistoPointer(const std::string &process, const std::string &distType)
{
    return getHistoPointer(process, distType, allProcHistoMap);
}

TH1F *TEventClass::getHistoPointerUnweighted(const std::string &process, const std::string &distType)
{
    return getHistoPointer(process, distType, allProcHistoMapUnweighted);
}

TH1F *TEventClass::getPDFHistoPointer(const std::string &process, const std::string distType,
                                      const std::string histname)
{
    return getHistoPointer(process, distType, histname, PDFvariedallProcHistoMap);
}

TH1F *TEventClass::getSystHistoPointer(const std::string &process, const std::string distType,
                                       const std::string systName)
{
    return getHistoPointer(process, distType, systName, SystematicsProcHistoMap);
}

TH2F *TEventClass::getResolutionsHistoPointer(std::string const &process, std::string distType)
{
    if (m_resolutionsMap.find(process) == m_resolutionsMap.end())
        return 0;
    if (m_resolutionsMap[process].find(distType) == m_resolutionsMap[process].end())
        return 0;
    return m_resolutionsMap[process][distType];
}

std::set<std::string> TEventClass::getSystematicNames()
{
    std::set<std::string> systSet = std::set<std::string>();
    for (auto name : m_systNames)
        systSet.insert(name);
    return systSet;
}

//----- Get Number of histograms per distType--------------------------------------------------

unsigned int TEventClass::getNumOfDistributions(
    const string process, std::map<std::string, std::map<std::string, std::map<std::string, TH1F *>>> &procMap)
{
    auto iter = procMap.find(process);
    if (iter == procMap.end())
        return 0;
    else
    {
        auto tempMap = iter->second.begin();
        return tempMap->second.size();
    }
}

unsigned int TEventClass::getNumOfSystHistos(const string &process)
{
    return getNumOfDistributions(process, SystematicsProcHistoMap);
}

unsigned int TEventClass::getNumOfPDFHistos(const string &process)
{
    return getNumOfDistributions(process, PDFvariedallProcHistoMap);
}

unsigned int TEventClass::getNumOfResolutionHistos(string const &process) const
{
    auto iter = m_resolutionsMap.find(process);
    if (iter == m_resolutionsMap.end())
        return 0;
    else
        return (*iter).second.size();
}

//--------------- Calculate PDF Uncertainty Histograms -----------------------------

void TEventClass::calculatePDFUncertainty()
{
    // Extract the right histograms and apply PDF reweighting.
    // loop over all processes
    std::map<std::string, std::vector<TH1F *>>::const_iterator PDFIter;
    std::string distType;
    std::string histoName;
    for (auto &procIter : allProcHistoMap)
    {
        std::string const &process = procIter.first;
        string histoNameBase = process + m_eventType + m_eventClassType;
        for (auto &distTypeMap : procIter.second)
        {
            distType = distTypeMap.first;
            std::vector<TH1F *> hists;
            // fill standard hist first
            hists.push_back(distTypeMap.second);
            // fill all corresponding PDF variations
            for (auto &PDFpair : PDFvariedallProcHistoMap[process][distType])
            {
                hists.push_back(PDFpair.second);
            }
            // Now there should be N+1 histograms, where N = "number of all used
            // PDF sets" (including alpha_s variations).
            if (hists.size() != m_numPDFvariations + 1)
            {
                std::stringstream err;
                err << "[ERROR] (TEventClass): ";
                err << "Number of histograms (" << hists.size() - 1 << ") ";
                err << "does not correspond to the number of PDF weights ";
                err << "(" << m_numPDFvariations << ")";
                err << std::endl;
                throw Tools::value_error(err.str());
            }
            applyReweighting(hists, process, distType);
            hists.clear();
        }
    }
}

//--------------- Apply Reweighting to histograms -----------------------------

void TEventClass::applyReweighting(std::vector<TH1F *> const &hists, std::string const &process, std::string distType)
{
    //~ vector< TH1F* > &histos = allProcHistoMap[ process ];

    TH1F *upper_hist;
    unsigned int num_bins = 0;
    num_bins = m_distTypeBinInfo[distType].size() - 1; // In m_*_bins the bin edges are defined, so one more than bins!
    upper_hist = SystematicsProcHistoMap[process][distType][m_histnamePDFup];

    if (upper_hist == 0)
        cout << "Empty upper hist class " << GetName() << " " << distType << " " << m_histnamePDFup << endl;
    // Loop over all bins in the histograms.
    for (unsigned int bin = 1; bin <= num_bins; ++bin)
    {
        // The first histogram is the actual distribution.
        // If there are no entries, in a bin, do not fill the weights vector, as
        // we don't need to calculate any uncertainties!
        if (hists.front()->GetBinContent(bin) == 0)
        {
            upper_hist->SetBinContent(bin, 0);
            continue;
        }
        // Loop over all PDF histograms (for this distribution) and extract the
        // weights for this bin.
        std::vector<float> weights;
        std::vector<TH1F *>::const_iterator hist;
        for (hist = hists.begin(); hist != hists.end(); ++hist)
        {
            weights.push_back((*hist)->GetBinContent(bin));
        }
        float const Uncert = MasterFormula(weights);

        // We do not use the upper/lower "method" anymore but calculate and store
        // the uncertainties directly.
        upper_hist->SetBinContent(bin, Uncert);
    }
    SystematicsProcHistoMap[process][distType][m_histnamePDFup] = upper_hist;
}

// Latest implementation of PDF4LHC.
// Following as closely as possible this one:
// https://arxiv.org/pdf/1510.03865.pdf
// http://www.hep.ucl.ac.uk/pdf4lhc/PDF4LHC_practical_guide.pdf
// (Using floats wherever possible to save memory.)
float TEventClass::MasterFormula(std::vector<float> &weights) const
{
    std::vector<PDFResult> results;

    // First one is from the production PDFs.
    float const prod_mean = weights.front();
    weights.erase(weights.begin());
    std::sort(weights.begin(), weights.end());
    // use weights which mark the beginning of the 16% and 84%  quantile
    unsigned int lower_index = std::round(weights.size() * 0.16);
    unsigned int upper_index = std::round(weights.size() * 0.84);

    float uncert = (weights[upper_index] - weights[lower_index]) / 2;

    return prod_mean + uncert;
}

// Delete all individual PDF histograms as they are not needed any longer (final
// merge).
void TEventClass::dropPDFHistograms()
{
    for (auto &procIter : PDFvariedallProcHistoMap)
    {
        for (auto &distTypeMap : procIter.second)
        {
            for (auto &histpair : distTypeMap.second)
            {
                delete histpair.second;
                histpair.second = 0;
            }
            distTypeMap.second.clear();
        }
    }
    PDFvariedallProcHistoMap.clear();
}

//--------------- For ROOT browsing -----------------------------
template <typename B, typename T>
void TEventClass::AddToFolder(B *parent, const char *name, std::map<std::string, T> &map) const
{
    const std::string newName = std::string(parent->GetName()) + std::string(name);
    // I hope that this object's ownership gets transferred to the parent
    // TFolder / TBrowser.
    TFolder *folder = new TFolder(name, newName.c_str());
    for (std::pair<std::string, T> pair : map)
    {
        AddToFolder(folder, pair.first.c_str(), pair.second);
    }
    parent->Add(folder);
}

template <typename B, typename T>
void TEventClass::AddToFolder(B *parent, const char *name, std::vector<T> objects) const
{
    TFolder *folder = new TFolder(name, name);
    for (int i = 0; i < objects.size(); ++i)
    {
        AddToFolder(folder, std::to_string(i).c_str(), objects[i]);
    }
    parent->Add(folder);
}

template <typename B> void TEventClass::AddToFolder(B *parent, const char *name, TObject *object) const
{
    parent->Add(object);
}

void TEventClass::Browse(TBrowser *browser)
{
    AddToFolder(browser, "processes", allProcHistoMap);
    AddToFolder(browser, "systematics", SystematicsProcHistoMap);
    AddToFolder(browser, "unweighted", allProcHistoMapUnweighted);
    AddToFolder(browser, "pdfVariations", PDFvariedallProcHistoMap);
    AddToFolder(browser, "resolutions", m_resolutionsMap);

    TFolder *dicedDataFolder = new TFolder("diced", "diced");
    for (const std::string &distType : getDistTypes())
    {
        if (hasDataScan(distType))
        {
            TFolder *distTypeFolder = new TFolder(distType.c_str(), distType.c_str());
            for (int i = 0; i < getNsignalRounds(distType); ++i)
            {
                TH1F *histo = new TH1F(getDicedDataHisto(distType, i));
                AddToFolder(distTypeFolder, std::to_string(i).c_str(), histo);
            }
            dicedDataFolder->Add(distTypeFolder);
        }
    }
    browser->Add(dicedDataFolder);
}

//------------------------------------------------------------------------------------------
std::string TEventClass::calculateEventClass(const std::map<std::string, int> countmap,
                                             const std::map<std::string, int> shortmap,
                                             std::function<bool(std::string, std::string)> orderFunction)
{

    std::map<std::string, int, std::function<bool(std::string, std::string)>> ordererd_map(orderFunction);
    // fill new map ordered
    ordererd_map.insert(countmap.begin(), countmap.end());

    std::ostringstream EventType;
    bool mapEmpty = true;
    for (auto &countpair : ordererd_map)
    {
        //~ std::cout<< countpair.first << " " << countpair.second << std::endl;
        if (countpair.second > 0)
        {
            std::string itemName = countpair.first;

            // check if a shortName map is present
            if (shortmap.find(countpair.first) != shortmap.end())
            {
                itemName = shortmap.at(countpair.first);
            }
            EventType << "_" << countpair.second << itemName;
            mapEmpty = false;
        }
    }
    if (mapEmpty)
        EventType << "_Empty";
    EventType.flush();
    return EventType.str();
}

void TEventClass::checkLumi(TEventClass *firstEC, TEventClass *secondEC)
{
    if (firstEC->getLumi() != secondEC->getLumi())
    {
        stringstream errorMessage;
        errorMessage << "In EventClass '" << secondEC->GetName() << "' ";
        errorMessage << "lumi in EventClass to be added (" << secondEC->getLumi() << ") != ";
        errorMessage << "lumi in base EventClass (" << firstEC->getLumi() << ")!";
        errorMessage << endl;
        throw Tools::value_error(errorMessage.str());
    }
}

void TEventClass::checkCrossSection(TEventClass *firstEC, TEventClass *secondEC, const string &process)
{
    if (firstEC->getCrossSection(process) != secondEC->getCrossSection(process))
    {
        stringstream errorMessage;
        errorMessage << "In EventClass '" << firstEC->GetName() << "' ";
        errorMessage << "in process '" << process << "' ";
        errorMessage << "cross section for process to be merged (" << secondEC->getCrossSection(process) << ") != ";
        errorMessage << "cross section for process in base EventClass (" << firstEC->getCrossSection(process) << ")!";
        errorMessage << endl;
        throw Tools::value_error(errorMessage.str());
    }
}

void TEventClass::rescalePDFHistograms(const string &process, const double &oldWeight, const double &newWeight)
{
    auto procIter = PDFvariedallProcHistoMap.find(process);
    if (procIter != PDFvariedallProcHistoMap.end())
    {
        for (auto &distTypeMap : procIter->second)
        {
            for (auto &histMap : distTypeMap.second)
            {
                histMap.second->Scale(1.0 / oldWeight);
                histMap.second->Scale(newWeight);
            }
        }
    }
}

void TEventClass::histNotFound(std::string classname, std::string process, std::string histname)
{
    stringstream errorMessage;
    errorMessage << "In EventClass '" << classname << "' ";
    errorMessage << "in process '" << process << "' ";
    errorMessage << "found invalid histogram  " << histname << ").";
    errorMessage << endl;
    throw runtime_error(errorMessage.str());
}

void TEventClass::mergeExistingProcess(std::string process, TEventClass *ECtoBeAdded, bool data)
{
    // If you do a parallel classification of the same process, the
    // EventClasses will end up in different files. You need to know the
    // actual number of all MC events (for one process) from all files,
    // if you want to do the normalisation right. This number must be
    // set from outside (i.e. in ECMerger2).
    //
    //~ std::cout << "addExistingEC " << process << std::endl;
    double totalEventsUnweightedNew =
        getTotalEventsUnweighted(process) + ECtoBeAdded->getTotalEventsUnweighted(process);
    double eventCountNew = getEventCount(process) + ECtoBeAdded->getEventCount(process);

    double scaleFactor = getScaleFactor(process);
    double scaleFactorToBeAdded = ECtoBeAdded->getScaleFactor(process);

    // The new scale factor is the Lumi * XSec / (total number of MC events in all classes that shall be merged in all
    // files).
    //
    double totalEventsNew = getLumi() * getCrossSection(process);
    double scaleFactorNew = totalEventsNew / totalEventsUnweightedNew;

    if (not data)
    {
        checkCrossSection(this, ECtoBeAdded, process);

        setEventCount(process, eventCountNew);
        setScaleFactor(process, scaleFactorNew);
        setTotalEvents(process, totalEventsNew);
    }
    for (auto &distTypeMap : allProcHistoMap[process])
    {
        TH1F *baseHist = distTypeMap.second;
        TH1F *mergeHist = ECtoBeAdded->getHistoPointer(process, distTypeMap.first);
        if (not baseHist)
            histNotFound(GetName(), process, distTypeMap.first);
        if (not mergeHist)
            histNotFound(ECtoBeAdded->GetName(), process, distTypeMap.first);
        if (not data)
        {
            // "Unscale" the histograms:
            //
            distTypeMap.second->Scale(1.0 / scaleFactor);
            mergeHist->Scale(1.0 / scaleFactorToBeAdded);
        }
        // Add the unscale histogram to the unscaled base histogram.
        //
        distTypeMap.second->Add(mergeHist, 1);
        // Add uweighted events
        allProcHistoMapUnweighted[process][distTypeMap.first]->Add(
            ECtoBeAdded->getHistoPointerUnweighted(process, distTypeMap.first));
        if (not data)
        {
            // And rescale it to the actual lumi.
            //
            distTypeMap.second->Scale(scaleFactorNew);
        }
    }

    // merge resolution hists
    for (auto resHistpair : m_resolutionsMap[process])
    {
        TH2F *baseResolutionHist = getResolutionsHistoPointer(process, resHistpair.first);
        TH2F *mergeResolutionHist = ECtoBeAdded->getResolutionsHistoPointer(process, resHistpair.first);
        if (not mergeResolutionHist)
            histNotFound(process, ECtoBeAdded->GetName(), "Resolution " + resHistpair.first);

        if (not data)
        {
            baseResolutionHist->Scale(1.0 / scaleFactor);
            mergeResolutionHist->Scale(1.0 / scaleFactorToBeAdded);
        }
        baseResolutionHist->Add(mergeResolutionHist, 1);

        if (not data)
        {
            baseResolutionHist->Scale(scaleFactorNew);
        }
    }

    //~ void TEventClass::addPDFtoEC( std::string process, TEventClass *ECtoBeAdded, bool data  ){
    if (not data)
    {

        // Add systematics
        // make sure that set of all sytematic names is updated
        std::set<std::string> addSystNames = ECtoBeAdded->getSystematicNames();
        m_systNames.insert(addSystNames.begin(), addSystNames.end());
        for (auto &distTypeMap : SystematicsProcHistoMap[process])
        {
            for (auto &histMap : distTypeMap.second)
            {
                TH1F *baseHist = histMap.second;
                TH1F *mergeHist = ECtoBeAdded->getSystHistoPointer(process, distTypeMap.first, histMap.first);
                if (not baseHist)
                    histNotFound(GetName(), process, distTypeMap.first);
                if (not mergeHist)
                    histNotFound(ECtoBeAdded->GetName(), process, distTypeMap.first + histMap.first);
                if (not data)
                {
                    // "Unscale" the histograms:
                    //
                    histMap.second->Scale(1.0 / scaleFactor);
                    mergeHist->Scale(1.0 / scaleFactorToBeAdded);
                }
                // Add the unscale histogram to the unscaled base histogram.
                //
                histMap.second->Add(mergeHist, 1);
                if (not data)
                {
                    // And rescale it to the actual lumi.
                    //
                    histMap.second->Scale(scaleFactorNew);
                }
            }
        }
        if (m_pdfInit)
        {
            // Now, merging the PDF histograms is a bit more tricky.
            // You can have EventClasses without PDF histograms, because due to
            // the parallel classification there can occur classes that have
            // only JES entries and nothing else.
            //
            std::map<std::string, std::map<std::string, TH1F *>> basePDFWeights = getPDFWeightsMap(process);
            std::map<std::string, std::map<std::string, TH1F *>> mergePDFWeights =
                ECtoBeAdded->getPDFWeightsMap(process);

            // If there are no PDF histograms in both classes: Nothin' to do here!
            //
            if (basePDFWeights.size() == 0 and mergePDFWeights.size() == 0)
                return;

            // If there are only PDF histograms in the base EventClass: Rescale them!
            //
            if (basePDFWeights.size() != 0 and mergePDFWeights.size() == 0)
            {
                rescalePDFHistograms(process, scaleFactor, scaleFactorNew);
            }
            // If there are *no* PDF histograms in the base classe but in the
            // class to be merged: Claim them and rescale them!
            //
            else if (basePDFWeights.size() == 0)
            {
                setPDFWeights(process, mergePDFWeights);
                // Take the scale factor from the class to be merged, since we
                // took the histograms from there too!
                //
                rescalePDFHistograms(process, scaleFactorToBeAdded, scaleFactorNew);
            }
            // If there are PDF histograms in both classes: Add up and rescale
            // them just like for the normal histograms!
            //
            else
            {
                for (auto &distTypeMap : PDFvariedallProcHistoMap[process])
                {
                    for (auto &histMap : distTypeMap.second)
                    {
                        TH1F *basePDFHist = histMap.second;
                        TH1F *mergePDFHist = ECtoBeAdded->getPDFHistoPointer(process, distTypeMap.first, histMap.first);
                        if (not basePDFHist or not mergePDFHist)
                        {
                            stringstream errorMessage;
                            if (not basePDFHist)
                                errorMessage << "In base EventClass '" << GetName() << "' ";
                            else
                                errorMessage << "In EventClass to be added '" << ECtoBeAdded->GetName() << "' ";
                            errorMessage << "in process '" << process << "' ";
                            errorMessage << "could not read PDF histogram " << histMap.first << " .";
                            errorMessage << endl;
                            throw runtime_error(errorMessage.str());
                        }
                        // "Unscale" the histograms:
                        //
                        histMap.second->Scale(1.0 / scaleFactor);
                        mergePDFHist->Scale(1.0 / scaleFactorToBeAdded);

                        // Add the unscaled histogram to the unscaled base histogram.
                        //
                        histMap.second->Add(mergePDFHist, 1);

                        // And rescale it to the actual lumi.
                        //
                        histMap.second->Scale(scaleFactorNew);
                    }
                }
            }
        }
    }
    // Merge change logs
    mergeChangeLogs(ECtoBeAdded->getChangeLog());
}

void TEventClass::mergeNewProcess(std::string process, TEventClass *ECtoBeAdded, bool data)
{
    // If the process does not exist in the base EventClass, we can simply add it.
    // But here it can happen, due to parallel classification, that there are
    // EventClasses only in one of the EventClass files. So, we also have to
    // take care of the normalisation here!
    //
    //~ std::cout << "addNewEC " << process << std::endl;

    double totalEventsUnweightedNew = getTotalEventsUnweighted(process);
    double eventCountNew = ECtoBeAdded->getEventCount(process);

    double scaleFactorToBeAdded = ECtoBeAdded->getScaleFactor(process);

    // The new scale factor is the Lumi * XSec / (total number of MC events in all classes that shall be merged in all
    // files).
    //
    double totalEventsNew = getLumi() * ECtoBeAdded->getCrossSection(process);
    double scaleFactorNew = totalEventsNew / totalEventsUnweightedNew;

    if (not data)
    {
        setEventCount(process, eventCountNew);
        setScaleFactor(process, scaleFactorNew);
        setTotalEvents(process, totalEventsNew);
        setCrossSection(process, ECtoBeAdded->getCrossSection(process));

        // Add systematics
        // make sure that set of all sytematic names is updated
        std::set<std::string> addSystNames = ECtoBeAdded->getSystematicNames();
        m_systNames.insert(addSystNames.begin(), addSystNames.end());
        // add histograms
        auto systematicsProcHistoMapAddedEC = ECtoBeAdded->getSystematicsProcHistoMap();
        for (auto &distTypeMap : systematicsProcHistoMapAddedEC[process])
        {
            for (auto &histMap : distTypeMap.second)
            {
                TH1F *addHist = histMap.second;
                if (not addHist)
                    histNotFound(GetName(), process, distTypeMap.first + " " + histMap.first);
                if (not data)
                {
                    // Take the histogram rescale it and add it to the (empty) base EventClass.
                    //
                    addHist->Scale(1.0 / scaleFactorToBeAdded);
                    addHist->Scale(scaleFactorNew);
                }
                addHisto(process, distTypeMap.first, histMap.first, (TH1F *)addHist->Clone());
            }
        }

        // add pdfs
        if (m_pdfInit)
        {
            // If there are PDF histograms, take them and add them to the (empty)
            // base EventClass.
            //
            std::map<std::string, std::map<std::string, TH1F *>> mergeWeights = ECtoBeAdded->getPDFWeightsMap(process);
            if (mergeWeights.size() > 0)
                setPDFWeights(process, mergeWeights);

            rescalePDFHistograms(process, scaleFactorToBeAdded, scaleFactorNew);
        }
        // add entry to process group map
        addToProcessGroupMap(process, ECtoBeAdded->getProcessGroup(process));
    }

    auto allProcHistoMapAddedEC = ECtoBeAdded->getAllProcHistoMap();
    for (auto &distTypeMap : allProcHistoMapAddedEC[process])
    {
        TH1F *addHist = distTypeMap.second;
        if (not addHist)
            histNotFound(GetName(), process, distTypeMap.first);
        addHistoUnweighted(process, distTypeMap.first, (TH1F *)addHist->Clone());
        if (not data)
        {
            // Take the histogram rescale it and add it to the (empty) base EventClass.
            //
            addHist->Scale(1.0 / scaleFactorToBeAdded);
            addHist->Scale(scaleFactorNew);
        }
        addHisto(process, distTypeMap.first, (TH1F *)addHist->Clone());
    }

    // add Resolution hists
    auto resolutionsMapAddedEC = ECtoBeAdded->getResolutionsMap();
    for (auto resolutionPair : resolutionsMapAddedEC[process])
    {
        TH2F *baseResolutionHist = getResolutionsHistoPointer(process, resolutionPair.first);
        if (baseResolutionHist)
        {
            stringstream errorMessage;
            errorMessage << "In EventClass '" << GetName() << "' ";
            errorMessage << "in process '" << process << "' ";
            errorMessage << "found existing resolution histogram for " << resolutionPair.first << ").";
            errorMessage << endl;
            throw runtime_error(errorMessage.str());
        }
        TH2F *mergeResolutionHist = resolutionPair.second;
        if (not data)
        {
            // Take the histogram rescale it and add it to the (empty) base EventClass.
            //
            mergeResolutionHist->Scale(1.0 / scaleFactorToBeAdded);
            mergeResolutionHist->Scale(scaleFactorNew);
        }

        addResolutionHistogram(process, resolutionPair.first, (TH2F *)mergeResolutionHist->Clone());
    }
    addToGlobalProcessList(process);

    // Merge change logs
    mergeChangeLogs(ECtoBeAdded->getChangeLog());
}

// This function can be used to add an EventClass (ECtoBeAdded) to an existing
// one. If the function finds a process that is already present in the base
// EventClass it merges it if it is found in the processesToMerge list.
// If it is not in the processesToMerge list, it ist skipped.
// If it is not present, it is added.
// WARNING: The PDF histograms are added and rescaled, but the overall PDF
// uncertainties are not recalculated (MasterFormula). This must be done from
// outside (i.e. in ECMerger2). This is due to performance issues, if you do it
// in here you do it once for every rootfile you have, this is inefficient!
void TEventClass::addEventClass(TEventClass *ECtoBeAdded, std::set<std::string> &processesToMerge, bool data)
{
    // Do not proceed if the lumis don't match!
    // (The user must take care of correct lumi normalisation.)
    //
    if (not data)
        checkLumi(this, ECtoBeAdded);

    // If the base TEventClass is empty, we check using the pdf init flag
    if (not m_pdfInit)
        m_pdfInit = ECtoBeAdded->getPDFInit();

    // When merging, we always want the smallest value,
    // except when the class is empty because then the min value is not defined.
    // Min. possible Sumpt is defined by the trigger cuts on the particles in the
    // trigger group and on the selection cuts on the objets.
    // Min. possible Minv is always 0.
    // Min. possible MET is defined by the trigger cuts (on MET) or the MET
    // selection cut. Thus, the min. MET can be different in the same EventClass
    // depending on the trigger stream it came from (consider something like
    // 1mu+1tau+MET).
    for (auto &distTopoMin : m_distTypeMins)
    {
        if (not ECtoBeAdded->isEmpty() and ECtoBeAdded->getDistTypeMinTopo(distTopoMin.first) < distTopoMin.second)
        {
            distTopoMin.second = ECtoBeAdded->getDistTypeMinTopo(distTopoMin.first);
        }
    }

    // If this is an empty EventClass, and the added one not, take the min. value
    // from the added one.
    if (isEmpty() and not ECtoBeAdded->isEmpty())
    {
        for (auto &distTopoMin : m_distTypeMins)
        {
            distTopoMin.second = ECtoBeAdded->getDistTypeMinTopo(distTopoMin.first);
        }
        for (auto &distReqMap : m_distTypeMinsRequire)
        {
            distReqMap.second = ECtoBeAdded->getDistTypeMinRequire(distReqMap.first);
        }
    }

    // The user required minimum values for Sumpt, Minv, MET should be identical
    // in all EventClasses that are not empty. I doesn't make sense to mix it in
    // any way.
    if (not isEmpty() and not ECtoBeAdded->isEmpty())
    {
        for (auto &distReqMap : m_distTypeMinsRequire)
        {
            if (distReqMap.second != ECtoBeAdded->getDistTypeMinRequire(distReqMap.first))
            {
                stringstream err;
                err << "In TEventClass::addEventClass(...):" << endl;
                err << "In EventClass '" << GetName() << "': ";
                err << "Required minimum values different in base EventClass and EventClass to be merged:" << endl;
                err << "BASE EC: " << distReqMap.first << " = " << distReqMap.second
                    << ", MERGE EC: " << ECtoBeAdded->getDistTypeMinRequire(distReqMap.first) << " = "
                    << ECtoBeAdded->getDistTypeMinRequire(distReqMap.first) << endl;
                err << "Please investigate!" << endl;
                throw Tools::value_error(err.str());
            }
        }
    }

    set<string> processList = getProcessList();
    set<string> processListToBeAdded = ECtoBeAdded->getProcessList();
    auto allProcHistoMapAddedEC = ECtoBeAdded->getAllProcHistoMap();

    // Loop over all processes in the input EventClass that shall be added to the
    // existing one.
    //
    bool procExists;
    for (auto &procIter : allProcHistoMapAddedEC)
    {
        procExists = false;
        std::string thisProc = procIter.first;
        // check if the process is present in current EC
        if (allProcHistoMap.find(thisProc) != allProcHistoMap.end())
        {
            procExists = true;
        }
        // merge existing classes or add new one otherwise
        if (procExists)
        {
            if (processesToMerge.count(thisProc) == 0)
            {
                std::cout << "Not in processesToMerge " << std::endl;
                continue;
            }
            mergeExistingProcess(thisProc, ECtoBeAdded, data);
        }
        else
        {
            mergeNewProcess(thisProc, ECtoBeAdded, data);
        }
    }
    m_isEmpty = false;
}
