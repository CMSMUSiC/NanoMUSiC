#ifndef PDFTOOL
#define PDFTOOL

#include <string>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "LHAPDF/LHAPDF.h"
#pragma GCC diagnostic pop

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"
#include "PDFInfo.hh"

namespace Tools
{
   class MConfig;
}

namespace pdf
{

   class PDFTool
   {
   public:
      typedef std::vector<std::string> vstring;
      typedef std::vector<unsigned int> vuint;
      typedef std::vector<LHAPDF::PDF *> PDFSets;
      PDFTool(Tools::MConfig const &config, unsigned int const debug = 1);
      ~PDFTool() {}

      // PDFInfo stores information about the read PDF sets.
      PDFInfo m_pdfInfo;
      // Delete old and write new PDF weights into the pxl::Event.
      void setPDFWeights(pxl::Event &event) const;
      pdf::PDFInfo const &getPDFInfo() const { return m_pdfInfo; }
      double calculatePdfAsUncertainty();
      void setPDFWeights(pxl::EventView const *genEvtView);
      std::vector<float> getPDFWeights() const { return m_pdf_weights; };
      std::pair<double, double> getAsWeights() const;

   private:
      void initPDFTool(pxl::EventView const *genEvtView);
      bool hasWeightsInView(pxl::EventView const *genEvtView);
      void initNNPDFRange(const std::string &name);
      void initNNPDFPDF();
      void setPDFWeightsLHAPDF(pxl::EventView const *genEvtView);
      double calcLHAWeight(float const Q,
                           float const x1,
                           float const x2,
                           int const f1,
                           int const f2,
                           double prodWeight,
                           LHAPDF::PDF *set);
      void setPDFWeightsFromView(pxl::EventView const *genEvtView);
      std::vector<float> parseArrayFromUserRecord(const std::string &pdfWeightsAsString);

      unsigned int const m_debug;
      // determination of the way to init / from view or from LHA needs first
      // event and can't be done on creation
      bool m_init;
      bool m_useLHAweights;
      std::string const m_pdfPath;
      std::string m_pdfProdName;
      LHAPDF::PDF const *m_pdfProd;
      PDFSets m_pdfSetsNNPDF;
      // up and down variation for NNPDF
      std::pair<LHAPDF::PDF *, LHAPDF::PDF *> m_AsSetsNNPDF;
      std::string NNPDFProd_set;
      // range of sets used for LO uncerts
      std::pair<int, int> m_NNPDFUncertIDRange;
      // two sets used for as scale variations
      std::pair<int, int> m_NNPDFAsUncertIDs;

      std::vector<std::string> m_PdfAsCombinedlabelOptions;
      // There are two ways of saving the pdf weights (table 5)
      // 1. Saving them in a vector with 102 entries
      // * mem 0-100: pdf weights for central alpha_S
      // * mem 101: alpha_S variation to 0.1165
      // * mem 102: alpha_S variation to 0.1195
      // 2. Saving a vector with 100 entries and two doubles
      std::string m_labelPdfAsCombined;
      std::string m_labelPdfAsCentral;
      std::string m_labelPdfAsDownShifted;
      std::string m_labelPdfAsUpShifted;
      bool m_pdfAsCombined;
      std::vector<float> m_pdf_weights;
      std::pair<double, double> m_As_weights;
   };

}

#endif /*PDFTOOL*/
