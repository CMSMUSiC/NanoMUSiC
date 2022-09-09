#ifndef PDFINFO
#define PDFINFO

// Simple container class for PDF related information.
// (Needed in TEventClass to calculate the actual PDF uncertainties!)
namespace pdf
{

struct PDFInfo
{
    PDFInfo() : init(false), n_cteq(0), n_mstw(0), n_nnpdf(0), n_alpha_cteq(0), n_alpha_mstw(0)
    {
    }

    unsigned int getNumPDFs() const
    {
        return n_cteq + n_mstw + n_nnpdf + n_alpha_cteq + n_alpha_mstw;
    }

    bool init;
    unsigned int n_cteq;
    unsigned int n_mstw;
    unsigned int n_nnpdf;
    unsigned int n_alpha_cteq;
    unsigned int n_alpha_mstw;
};

} // namespace pdf

#endif /*PDFINFO*/
