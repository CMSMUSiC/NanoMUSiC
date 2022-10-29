#include "PDFTool.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Tools/MConfig.hpp"
#include "Tools/Tools.hpp"

using namespace pdf;

PDFTool::PDFTool(Tools::MConfig const &config, unsigned int const debug)
    : m_pdfInfo(), m_debug(debug), m_init(false), m_useLHAweights(false),
      // LHAPATH is an environment variable that is set, when you source
      // '/cvmfs/cms.cern.ch/cmsset_default.sh' and call 'cmsenv'.
      // If it is set correctly, LHAPDF will find the files containing the PDF sets.
      m_pdfPath("/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current"),
      // The name of the PDF set with which your sample has been produced. You can set it in some config file.
      m_pdfProdName(config.GetItem<std::string>("PDF.Prod.Name")), m_pdfProd(LHAPDF::mkPDF(m_pdfProdName, 0)),
      m_pdfSetsNNPDF(PDFSets()), m_AsSetsNNPDF(std::make_pair(nullptr, nullptr)),
      m_PdfAsCombinedlabelOptions(
          Tools::splitString<std::string>(config.GetItem<std::string>("PDF.PDFAsCombinedLabels"), true)),
      // The name of one or more PDF sets that contain As weights in themselves.
      // You can set them in some config file e.g. PxlAnalyzer/ConfigFiles/pdf.cff.
      // Below are the name of PDF sets if the PDF central DOES NOT CONTAINS As weights in itself, but As are provided
      // different PDF sets.
      m_labelPdfAsCombined(""),
      m_labelPdfAsCentral(config.GetItem<std::string>("PDF.PDFAsCentral.Name", "NNPDF31_nnlo_as_0118")),
      m_labelPdfAsDownShifted(config.GetItem<std::string>("PDF.PDFAsDownShifted.Name", "NNPDF31_nnlo_as_0116")),
      m_labelPdfAsUpShifted(config.GetItem<std::string>("PDF.PDFAsUpShifted.Name", "NNPDF31_nnlo_as_0120")),
      m_pdfAsCombined(false)
// initNNPDFPDFLO( config ) )
{
}

void PDFTool::initPDFTool(pxl::EventView const *genEvtView)
{
    // check if PDFinfos are saved in event view
    if (not hasWeightsInView(genEvtView))
    {
        initNNPDFRange(m_pdfProdName);
        initNNPDFPDF();
        m_useLHAweights = true;
    }
    m_init = true;
    m_pdfInfo.init = true;
}

// Check if weights are given in sample
// if they are given check if as uncert is combined for set or saved
// in m_pdfAsCombined as bool.
bool PDFTool::hasWeightsInView(pxl::EventView const *genEvtView)
{
    // check consitency
    // check if this set uses a combined set
    for (const auto &pdfAslabel : m_PdfAsCombinedlabelOptions)
    {
        if (genEvtView->hasUserRecord(pdfAslabel))
        {
            m_labelPdfAsCombined = pdfAslabel;
            m_pdfAsCombined = true;
            std::cout << " pdfAslabel is in the sample....  so   hasWeightsInView returns True.......  " << std::endl;
            return true;
        }
    }
    if (genEvtView->hasUserRecord(m_labelPdfAsCentral))
    {
        m_pdfAsCombined = false;
        if (!genEvtView->hasUserRecord(m_labelPdfAsDownShifted) or !genEvtView->hasUserRecord(m_labelPdfAsUpShifted))
        {
            std::stringstream err;
            err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
            err << "No values for alpha s variation given!" << std::endl;
            throw Tools::value_error(err.str());
        }
        std::cout << " pdfAsCentral   is in the sample....  so   hasWeightsInView return True.......  " << std::endl;
        return true;
    }
    std::cout << "hasWeightsInView returns False.......  " << std::endl;
    return false;
}

// Init range of LHAIds which should be used dependent on the central (production) set
// taken from:
// https://indico.cern.ch/event/459797/contribution/2/attachments/1181555/1800214/mcaod-Feb15-2016.pdf
// Slide 13
void PDFTool::initNNPDFRange(const std::string &name)
{
    if (name == "NNPDF23_lo_as_0130_qed")
    {
        m_NNPDFUncertIDRange = std::make_pair(247001, 247100);
        m_NNPDFAsUncertIDs = std::make_pair(0, 0);
    }
    else if (name == "NNPDF23_nlo_as_0118_qed")
    {
        m_NNPDFUncertIDRange = std::make_pair(244601, 244700);
        m_NNPDFAsUncertIDs = std::make_pair(244400, 244800);
    }
    else if (name == "NNPDF30_lo_as_0130")
    {
        m_NNPDFUncertIDRange = std::make_pair(263001, 263100);
        m_NNPDFAsUncertIDs = std::make_pair(0, 0);
    }
    else if (name == "NNPDF31_lo_as_0130")
    {
        m_NNPDFUncertIDRange = std::make_pair(315201, 315300);
        m_NNPDFAsUncertIDs = std::make_pair(0, 0);
    }
    else if (name == "NNPDF30_lo_as_0130_nf_4")
    {
        m_NNPDFUncertIDRange = std::make_pair(263401, 263500);
        m_NNPDFAsUncertIDs = std::make_pair(0, 0);
    }
    else if (name == "NNPDF30_nlo_as_0118")
    {
        m_NNPDFUncertIDRange = std::make_pair(260001, 260100);
        m_NNPDFAsUncertIDs = std::make_pair(265000, 266000);
    }
    else if (name == "NNPDF30_nlo_as_0118_nf_4")
    {
        m_NNPDFUncertIDRange = std::make_pair(260401, 260500);
        m_NNPDFAsUncertIDs = std::make_pair(265400, 266400);
    }
    else if (name == "NNPDF30_nlo_nf_5_pdfas")
    {
        m_NNPDFUncertIDRange = std::make_pair(292201, 292300);
        m_NNPDFAsUncertIDs = std::make_pair(292301, 292302);
    }
    else if (name == "NNPDF30_nlo_nf_4_pdfas")
    {
        m_NNPDFUncertIDRange = std::make_pair(292201, 292300);
        m_NNPDFAsUncertIDs = std::make_pair(292101, 292102);
    }
    else if (name == "CT10")
    {
        m_NNPDFUncertIDRange = std::make_pair(10801, 10852);
        m_NNPDFAsUncertIDs = std::make_pair(0, 0);
    }
    else if (name == "NNPDF31_nnlo_as_0118") // Added by Sebastian to follow 2017 recommendations. See above where to
                                             // find the latest once
    {
        m_NNPDFUncertIDRange = std::make_pair(303601, 303700);
        // Modified by Lorenzo to have alphas variations.
        // These IDs correspond to NNPDF31_nnlo_as_0116 and NNPDF31_nnlo_as_0120.
        // Default pair is (0 , 0)
        m_NNPDFAsUncertIDs = std::make_pair(319300, 319500);
    }
    else
    {
        m_NNPDFUncertIDRange = std::make_pair(0, 0);
        m_NNPDFAsUncertIDs = std::make_pair(0, 0);
    }
}

void PDFTool::initNNPDFPDF()
{
    LHAPDF::setPDFPath(m_pdfPath);

    // We know there will be ~50-100 PDFSets to store.
    m_pdfSetsNNPDF.reserve(100);

    for (int setID = m_NNPDFUncertIDRange.first; setID <= m_NNPDFUncertIDRange.second; setID++)
    {
        m_pdfSetsNNPDF.push_back(LHAPDF::mkPDF(setID));
    }
    m_pdfInfo.n_nnpdf = m_pdfSetsNNPDF.size();
    if (m_NNPDFAsUncertIDs.first and m_NNPDFAsUncertIDs.second)
    {
        m_AsSetsNNPDF =
            std::make_pair(LHAPDF::mkPDF(m_NNPDFAsUncertIDs.first), LHAPDF::mkPDF(m_NNPDFAsUncertIDs.second));
    }
}

void PDFTool::setPDFWeightsLHAPDF(pxl::EventView const *genEvtView)
{

    // clear already existing weights
    m_pdf_weights.clear();

    float const Q = genEvtView->getUserRecord("Generator_scalePDF");
    float const x1 = genEvtView->getUserRecord("Generator_x1");
    float const x2 = genEvtView->getUserRecord("Generator_x2");
    int const f1 = genEvtView->getUserRecord("Generator_id1");
    int const f2 = genEvtView->getUserRecord("Generator_id2");
    double prodWeight = m_pdfProd->xfxQ(f1, x1, Q) * m_pdfProd->xfxQ(f2, x2, Q);

    for (auto &PDFSet : m_pdfSetsNNPDF)
    {
        m_pdf_weights.push_back(calcLHAWeight(Q, x1, x2, f1, f2, prodWeight, PDFSet));
    }
    double as_weight_down = 0;
    double as_weight_up = 0;
    if (m_AsSetsNNPDF.first && m_AsSetsNNPDF.second)
    {
        as_weight_down = calcLHAWeight(Q, x1, x2, f1, f2, prodWeight, m_AsSetsNNPDF.first);
        as_weight_up = calcLHAWeight(Q, x1, x2, f1, f2, prodWeight, m_AsSetsNNPDF.second);
        if (m_debug > 3)
        {
            std::cout << "In  setPDFWeightsLHAPDF ::::::::  " << std::endl;
            std::cout << "if( m_AsSetsNNPDF.first && m_AsSetsNNPDF.second )    "
                      << " as_weight_down:  " << as_weight_down << "  as_weight_up  " << as_weight_up << std::endl;
        }
    }
    m_As_weights = std::make_pair(as_weight_down, as_weight_up);
}

double PDFTool::calcLHAWeight(float const Q, float const x1, float const x2, int const f1, int const f2,
                              double prodWeight, LHAPDF::PDF *set)
{
    // Compute the PDF weight for this event.
    double pdfWeight = set->xfxQ(f1, x1, Q) * set->xfxQ(f2, x2, Q);

    // Divide the new weight by the weight from the PDF the event was produced with.
    return pdfWeight / prodWeight;
}

void PDFTool::setPDFWeightsFromView(pxl::EventView const *genEvtView)
{
    std::vector<double> pdfWeights;
    // clear existing entries in pdfweights
    m_pdf_weights.clear();
    double as_weight_down = 0;
    double as_weight_up = 0;

    // get generator weight, since pdfweights in evtview are pdfweights * eventweights
    if (!genEvtView->hasUserRecord("Weight"))
    {
        std::stringstream err;
        err << "In file " << __FILE__ << ", function " << __func__ << ", line " << __LINE__ << std::endl;
        err << "User Record 'Weight' not found but expected!" << std::endl;
        throw Tools::value_error(err.str());
    }
    double generator_weight = genEvtView->getUserRecord("Weight");

    if (m_pdfAsCombined)
    {
        std::string pdfWeightsAsString = genEvtView->getUserRecord(m_labelPdfAsCombined);
        m_pdf_weights = parseArrayFromUserRecord(pdfWeightsAsString);
        if (m_pdf_weights.size() == 103)
        {
            std::cout << "m_pdf_weights.size() == 103   ------->   m_pdf_weights.erase( m_pdf_weights.begin() )   "
                      << std::endl;
            m_pdf_weights.erase(m_pdf_weights.begin());
        }
        as_weight_down = m_pdf_weights.at(100);
        as_weight_up = m_pdf_weights.at(101);
        // erase elemts from vector as they should not be part of normal uncerts anymore
        m_pdf_weights.erase(m_pdf_weights.begin() + 101);
        m_pdf_weights.erase(m_pdf_weights.begin() + 100);
        if (m_debug > 3)
        {
            std::cout << "In setPDFWeightsFromView    and       m_pdfAsCombined is True   " << std::endl;
            std::cout << "m_pdfAsCombined    "
                      << " as_weight_down:  " << as_weight_down << "  as_weight_up  " << as_weight_up << std::endl;
        }
    }
    else // not combined
    {
        std::string pdfWeightsAsString = genEvtView->getUserRecord(m_labelPdfAsCentral);
        m_pdf_weights = parseArrayFromUserRecord(pdfWeightsAsString);
        if (m_pdf_weights.size() == 101)
            m_pdf_weights.erase(m_pdf_weights.begin());
        as_weight_down = std::stod(genEvtView->getUserRecord(m_labelPdfAsDownShifted));
        as_weight_up = std::stod(genEvtView->getUserRecord(m_labelPdfAsUpShifted));
        if (m_debug > 3)
        {
            std::cout << "In setPDFWeightsFromView" << std::endl;
            std::cout << " NOT m_pdfAsCombined    "
                      << " as_weight_down:  " << as_weight_down << "  as_weight_up  " << as_weight_up << std::endl;
        }
    }
    for (auto &pdf_weight : m_pdf_weights)
    {
        pdf_weight /= generator_weight;
    }
    m_As_weights = std::make_pair(as_weight_down / generator_weight, as_weight_up / generator_weight);
    if (m_debug > 3)
    {
        std::cout << " In setPDFWeightsFromView     the weight pair you store at the end is as_weight / "
                     "generator_weight ..... so    "
                  << " as_weight_down:  " << m_As_weights.first << "  as_weight_up  " << m_As_weights.second
                  << std::endl;
        std::cout << "*** generator_weight is *** = " << generator_weight << std::endl;
    }
}

// call this function once per event to retrieve weight infos from matching source
void PDFTool::setPDFWeights(pxl::EventView const *genEvtView)
{
    // check if we need to init the PDFtool ( happens in first event )
    if (not m_init)
        initPDFTool(genEvtView);
    // retireve weights
    m_useLHAweights ? setPDFWeightsLHAPDF(genEvtView) : setPDFWeightsFromView(genEvtView);
    m_pdfInfo.n_nnpdf = m_pdf_weights.size();
}

std::pair<double, double> PDFTool::getAsWeights() const
{
    return std::make_pair(m_As_weights.first, m_As_weights.second);
}

// Retrieve pdf weights from user Record which is a string of doubles seperated by ';'
std::vector<float> PDFTool::parseArrayFromUserRecord(const std::string &pdfWeightsAsString)
{
    std::string delimiter = ";";

    // get PDF weights
    std::vector<float> pdfWeights;

    unsigned int pos1 = 0;
    unsigned int pos2 = 0;
    // first: string ending with number, second: string ending with delimiter
    while (pos2 != std::string::npos && pos1 != pdfWeightsAsString.length())
    {
        pos2 = pdfWeightsAsString.find(delimiter, pos1);
        // string bla = pdfWeightsAsString.substr( pos1, pos2 - pos1 );
        // cout <<  "pdfWeightsAsString.length():  "<< pdfWeightsAsString.length() << " " << pos1 << " " << pos2 << " "
        // << bla << std::endl;
        pdfWeights.push_back(std::stof(pdfWeightsAsString.substr(pos1, pos2 - pos1)));
        pos1 = pos2 + 1; // + delimiter.length();
    }

    return pdfWeights;
}
