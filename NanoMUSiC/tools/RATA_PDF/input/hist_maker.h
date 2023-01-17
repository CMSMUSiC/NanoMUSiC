/** \file hist_maker.h
 *
 * \brief Functions to calculate the PDF uncertainties.
 * The functions will be called by tha RATA_PDF package.
 *
 */

#include <TROOT.h>
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <iostream>
#include <fstream>
#include "TSystem.h"
#include <TChain.h>
#include <TFile.h>
#include <TH1.h>
#include <TEfficiency.h>
#include <THnSparse.h>
#include <TKey.h>
#include <vector>
#include <map>
#include <TStopwatch.h>
#include <string>
//#include "RunPDFErrors.h"

#include "LHAPDF/LHAPDF.h"

using namespace std;

// global Variable to log errors (1), warnings (2), info (3), debug(4,5,...)
int gLogLevel = 4; /*!< Logging level, coded as an integer (errors=1, warnings=2, info=3, debug=4,5,...) */

#ifndef NDEBUG
#define LOG(level, message) { if (gLogLevel >= level) { switch (level) { \
case 1: std::cerr << "ERROR: " << message << std::endl; break; \
case 2: std::cerr << "WARNING: " << message << std::endl; break; \
case 3: std::cout << "INFO: " << message << std::endl; break; \
default: std::cout << "DEBUG: " << message << std::endl; } } }
#else
#define LOG(level, message) ;
#endif

#define ERROR(message) LOG(1, message);
#define WARNING(message) LOG(2, message);
#define INFO(message) LOG(3, message);
#define DEBUG(message) LOG(10000, message);

// throw an exception that tells me where the exception happened
#define THROW(errmsg) throw (std::string( __PRETTY_FUNCTION__ )+std::string(" (file: ")+std::string( __FILE__ )+std::string(", line: ")+std::string( Form("%d", __LINE__) )+std::string(") ")+std::string(errmsg));


ProcInfo_t info; /*!< Process info, to get present memory usage etc. */

/*! \brief Function to simulate raw_input from python
 *
 * This function prints a message and waits for user input.
 * \param[in] question TString of the message that should be printed
 */
void raw_input(TString question);

/*! \brief Function to read the MC scale factor from the number of events
 *
 * This function takes the luminosity, data/MC scale factor and the
 * cross section for the process, reads the number of events from a
 * given files and returns the scale factor for this sample.
 * \param[in] f_name Name of the file from which the number of events should be read
 * \param[in] lumi Luminosity
 * \param[in] data_mc_scf Optional scale factor between MC and data
 * \param[in] xs Cross section of the process
 * \param[out] scf Scale factor for the given sample
 */
double get_scalefactor(char* f_name,double lumi,double data_mc_scf,double xs);

/*! \brief Function to initialze the necessary PDF sets
 *
 * This function is called by RATA_PDF to initiate the PDF sets,
 * used later for the calculation of the PDF uncertainties.
 * \param[in] PDFpath Path to where the PDF sets are installed
 * \param[in] n_pdfs Number of PDF sets that should be initialized
 * \param[in] PDFsets Array of the PDF set names that should be initialized
 * \param[in] loglevel Coded logging level
 */
extern "C" void init_bg(char* PDFpath, int n_pdfs, char** PDFsets, int loglevel);

/*! \brief Function to read and cut a tree from a .root file
 *
 * This function reads in a TTree from a given .root files,
 * applies given cuts and stores the cutted tree in the memory
 * \param[in] filename Name of the file from which the tree should be read
 * \param[in] tree_name Name of the TTree that should be read
 * \param[in] tree_cuts String of cuts that should be applied
 * \param[out] smallerTree Final read in and cutted tree
 */
TTree* read_tree(char* filename, char* tree_name, char* tree_cuts);

/*! \brief Function to create for a given TTree the reweighted histograms
 *
 * This function reads the necessary information from a given TTree and
 * reweights all event according to the specified PDF sets. This events
 * are then filled in histograms fro further processing.
 * \param[in] tree TTree that should be analyzed
 * \param[in] branches Array with the name of the branches in the tree
 * \param[in] histname Name that all filled histograms should get
 * \param[in] n_pdfs Number of PDF sets to be analyzed
 * \param[in] PDFsets Array with the name of the PDF sets to be analyzed
 * \param[in] scalefactor Scalefactor that should be applied to the histograms
 * \param[out] bool Either true(successfull loop) or false(something failed)
 */
bool analyser(TTree* tree, char** branches, char* histname, int n_pdfs, char** PDFsets, double scalefactor);

/*! \brief Function to write the histograms in an output file
 *
 * This function writes the histograms in an output files, and checks
 * the bin content of each histogram, negative bin contents are set to
 * zero.
 * \param[in] histname Name that all filled histograms have
 * \param[in] n_pdfs Number of PDF sets to be analyzed
 * \param[in] PDFsets Array with the name of the PDF sets to be analyzed
 * \param[out] bool Either true(successfull writing) or false(something failed)
 */
bool writer(char* histname, int n_pdfs, char** PDFsets);

/*! \brief Function to coordinate the creation of the reweighted histograms
 *
 * This function prints if necessary the most important information on
 * the reweighting process. It creates the output file an initialzes the
 * necessary histograms with the user defined binning. It calls the
 * analyser and the writer and cleans all histograms and files in the
 * end. This function is also called by RATA_PDF.
 * \param[in] input_file Name of the file from which the tree should be read
 * \param[in] tree_name Name of the TTree that should be read
 * \param[in] tree_cuts String of cuts that should be applied to the tree
 * \param[in] branches Array with the name of the branches in the tree
 * \param[in] lumi Luminosity
 * \param[in] cross_section Cross section of the process
 * \param[in] n_pdfs Number of PDF sets to be analyzed
 * \param[in] PDFsets Array with the name of the PDF sets to be analyzed
 * \param[in] PDFnorm Name of the PDF set which should be used for the normalization
 * \param[in] histname Name that all filled histograms should have
 * \param[in] output_file Name that the output file should get
 * \param[in] n_binning Number of bins that each histogram should have
 * \param[in] binning Array with the bin edges for the histograms
 */
extern "C" void make_hists(char* input_file, char* tree_name, char* tree_cuts, char** branches, double lumi, double cross_section, int n_pdfs, char** PDFsets, char* PDFnorm, char* histname, char* output_file, int n_binning, double* binning);

/*! \brief Function to calculate the overall uncertainty for a hessian PDF set
 *
 * This function calculates the overall uncertainty for a hessian PDF
 * set. It also calculates the uncertainties due to the alpha_S and
 * adds this two uncertainties in quadrature. The whole procedure follows
 * the PDF4LHC recommendation.
 * \param[in] hist_scaled Vector of Vector of all histograms necessary
 * \param[in] pdf_correction Global correction factor for this PDF set
 * \param[in] as_plus_correction Correction factor for the alpha_S plus uncertainties
 * \param[in] as_minus_correction Correction factor for the alpha_S minus uncertainties
 * \param[in] as_plus_number Number of the alpha_S plus member, defined by the PDF config
 * \param[in] as_minus_number Number of the alpha_S minus member, defined by the PDF config
 * \param[out] v_as_errors Vector of Vector with the resulting uncertainties for each bin
 */
vector< vector<Float_t> > hessian_pdf_asErr(vector< vector< TH1D* > > hist_scaled, double pdf_correction, double as_plus_correction, double as_minus_correction, int* as_plus_number, int* as_minus_number);

/*! \brief Function to get the final uncertainty histograms for a hessian PDF set
 *
 * This function reads in the necessary reweighted histograms, calls 
 * hessian_pdf_asErr and structures the output in a nice histogram
 * format. This function is called by RATA_PDF.
 * \param[in] n_pdfSet Number of PDF sets to be analyzed
 * \param[in] pdf_sets Array with the name of the PDF sets to be analyzed
 * \param[in] outfile Name that the output file should get
 * \param[in] out_par How to handle the output file (RECREATE, UPDATE, ...)
 * \param[in] infile Name of the input file
 * \param[in] main_hist Base name of all histograms
 * \param[in] shortname Short name that the output histograms should get
 * \param[in] pdf_correction Global correction factor for this PDF set
 * \param[in] as_plus_correction Correction factor for the alpha_S plus uncertainties
 * \param[in] as_minus_correction Correction factor for the alpha_S minus uncertainties
 * \param[in] as_plus_number Number of the alpha_S plus member, defined by the PDF config
 * \param[in] as_minus_number Number of the alpha_S minus member, defined by the PDF config
 */
extern "C" void pdf_calcer_hessian(int n_pdfSet, char** pdf_sets, char* outfile, char* out_par, char* infile, char* main_hist, char* shortname, double pdf_correction, double as_plus_correction, double as_minus_correction, int* as_plus_number, int* as_minus_number);

/*! \brief Function to calculate the overall uncertainty for a MC PDF set
 *
 * This function calculates the overall uncertainty for a MC PDF
 * set directly combined with the uncertainties due to alpha_S. The
 * whole procedure follows the PDF4LHC recommendation.
 * \param[in] hist_scaled Vector of Vector of all histograms necessary
 * \param[out] v_as_errors Vector of Vector with the resulting uncertainties for each bin
 */
vector< vector<Float_t> > NNPDF_weighted_mean( vector< vector< TH1D* > > hist_scaled);

/*! \brief Function to get the final uncertainty histograms for a MC PDF set
 *
 * This function reads in the necessary reweighted histograms, calls 
 * NNPDF_weighted_mean and structures the output in a nice histogram
 * format. This function is called by RATA_PDF.
 * \param[in] n_pdfSet Number of PDF sets to be analyzed
 * \param[in] pdf_sets Array with the name of the PDF sets to be analyzed
 * \param[in] outfile Name that the output file should get
 * \param[in] out_par How to handle the output file (RECREATE, UPDATE, ...)
 * \param[in] infile Name of the input file
 * \param[in] main_hist Base name of all histograms
 * \param[in] shortname Short name that the output histograms should get
 */
extern "C" void pdf_calcer_MC(int n_pdfSet, char** pdf_sets, char* outfile, char* out_par, char* infile, char* main_hist, char* shortname);

LHAPDF::PDF* Mainpdf; /*!< PDF set that will be used for normalization. */

vector<vector< LHAPDF::PDF* > > allPdfsets; /*!< Vector of Vector of all necessary PDF sets. */

static map<TString, TH1D * > histo; /*!< Map of a string and histogram, for easy histogram handling. */

/** \namespace HistClass
 *
 * \brief Functions to have easy histogram handling
 *
 * In this namespace different functions to interact with the histogram
 * map are included, to create, fill and write histograms in a convinient
 * way.
 */
namespace HistClass
{
    /*! \brief Function to create histograms in the histo map
     *
     * \param[in] n_histos Number of histograms that should be created with different numbers
     * \param[in] name Name of the histograms that should be created
     * \param[in] title Title of the histograms that should be created
     * \param[in] nbinsx Number of bins on the x-axis
     * \param[in] xlow Lower edge of the x-axis
     * \param[in] xup Upper edge of the x-axis
     * \param[in] xtitle Optinal title of the x-axis (DEFAULT = "")
     */
    static void CreateHisto(Int_t n_histos, const char* name, const char* title, Int_t nbinsx, Double_t xlow, Double_t xup, TString xtitle = "")
    {
        for(int i = 0; i < n_histos; i++){
            string dummy = Form("h1_%d_%s", i, name);
            histo[dummy] = new TH1D(Form("h1_%d_%s", i, name), title, nbinsx, xlow, xup);
            histo[dummy] -> Sumw2();
            histo[dummy] -> GetXaxis() ->SetTitle(xtitle);
        }
    }

    /*! \brief Function to clear histograms in the histo map
     *
     * \param[in] n_histos Number of histograms with the same name
     * \param[in] name Name of the histograms that should be cleared
     */
    static void ClearHisto(Int_t n_histos, const char* name)
    {
        for(int i = 0; i < n_histos; i++){
            string dummy = Form("h1_%d_%s", i, name);
            histo[dummy] -> Reset();
        }
    }

    /*! \brief Function to rebin histograms in the histo map
     *
     * \param[in] n_histos Number of histograms with the same name
     * \param[in] name Name of the histograms that should be rebinned
     * \param[in] n_rebin Number of bins that the rebinned histogram should have
     * \param[in] bins Array of bin edges that the rebinned histogram should have
     */
    static void RebinHisto(Int_t n_histos, const char* name, Int_t n_rebin, Double_t* bins)
    {
        for(int i = 0; i < n_histos; i++){
            string dummy = Form("h1_%d_%s", i, name);
            char* cdummy = Form("h1_%d_%s", i, name);
            histo[dummy] = (TH1D*)histo[dummy] -> Rebin(n_rebin,cdummy, bins);
        }
    }

    /*! \brief Function to fille events in a histogram in the histo map
     *
     * \param[in] n_histos Number of the histograms to be filled
     * \param[in] name Name of the histogram to be filled
     * \param[in] value Value that should be filled in the histogram
     * \param[in] weight Weight the the event should get
     */
    static void Fill(Int_t n_histo, const char * name, double value, double weight)
    {
      string dummy = Form("h1_%d_%s", n_histo, name);
      histo[dummy]->Fill(value,weight);
    }

    /*! \brief Function to write a histogram from the histo map
     *
     * \param[in] n_histos Number of histogram to be written
     * \param[in] name Name of the histogram to be written
     */
    static void Write(Int_t n_histo, const char * name)
    {
      string dummy = Form("h1_%d_%s", n_histo, name);
      histo[dummy]->Write();
    }

    /*! \brief Function to set every negative bin content to zero
     *
     * \param[in] n_histos Number of histogram to be adapted
     * \param[in] name Name of the histogram to be adapted
     */
    static void SetToZero(Int_t n_histo, const char * name)
    {
      string dummy = Form("h1_%d_%s", n_histo, name);
      int Nbins2 = histo[dummy] -> GetNbinsX();
      for ( int bb = 0; bb < Nbins2+1; bb++) {
        double binValue = histo[dummy] -> GetBinContent(bb);
        if (binValue < 0) {
          //cout << "Bin " << bb << "  " << dummy << " is negative: " << binValue << "  and is being set to zero!" << endl;
          histo[dummy] -> SetBinContent(bb,0.);
        }
      }
    }

    /*! \brief Function to get a histogram from the histo map
     *
     * \param[in] n_histos Number of histogram to be read
     * \param[in] name Name of the histogram to be read
     */
    static TH1D* ReturnHist(Int_t n_histo, const char * name)
    {
      string dummy = Form("h1_%d_%s", n_histo, name);
      return histo[dummy];
    }

    /*! \brief Function to delete a histogram from the histo map
     *
     * \param[in] n_histos Number of histogram to be deleted
     * \param[in] name Name of the histogram to be deleted
     */
    static void DeleteHisto(Int_t n_histos, const char* name){
        for(int i = 0; i < n_histos; i++){
            string dummy = Form("h1_%d_%s", i, name);
            delete histo[dummy];
        }
    }
}
