#ifndef PDF_ALPHA_S
#define PDF_ALPHA_S

#include "fmt/format.h"

#include <exception>
#include <optional>
#include <regex>

#include <fmt/format.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "LHAPDF/LHAPDF.h"
#pragma GCC diagnostic pop

#include <TTree.h>

// References:
// clang-format off
//  https://cms-pdmv.gitbook.io/project/mccontact/info-for-mc-production-for-ultra-legacy-campaigns-2016-2017-2018#recommendations-on-the-usage-of-pdfs-and-cpx-tunes
// https://indico.cern.ch/event/494682/contributions/1172505/attachments/1223578/1800218/mcaod-Feb15-2016.pdf
// https://twiki.cern.ch/twiki/bin/view/CMS/TopSystematics
// clang-format on

namespace PDFAlphaSWeights
{
// struct LHAPDFInfo{unsigned int id; std::string_view name; ErrorType};

//     std::unordered_map<unsigned int, LHAPDFInfo> lhaids =

inline bool has_pdf_weight(TTree *events_tree)
{
    if (events_tree->GetBranch("LHEPdfWeight"))
    {
        return true;
    }
    return false;
}

// will look for IDs in the LHEPdfWeight description string
// if can not find then, will return a std::nullopt
inline std::optional<std::pair<unsigned int, unsigned int>> get_pdf_ids(TTree *events_tree)
{
    std::optional<std::pair<unsigned int, unsigned int>> ids = std::nullopt;
    if (has_pdf_weight(events_tree))
    {
        std::string LHEPdfWeight_description = events_tree->GetBranch("LHEPdfWeight")->GetTitle();

        if (LHEPdfWeight_description.empty())
        {
            return ids;
        }

        std::regex expression("[0-9]+");

        // default constructor = end-of-sequence:
        std::regex_token_iterator<std::string::iterator> regex_end;
        std::regex_token_iterator<std::string::iterator> regex_iter(
            LHEPdfWeight_description.begin(), LHEPdfWeight_description.end(), expression);
        auto match_counter = 0;
        std::array<std::string, 2> buffer_ids = {"0", "0"};

        while (regex_iter != regex_end)
        {
            buffer_ids[match_counter] = *regex_iter;
            regex_iter++;
            match_counter++;
        }
        if (match_counter != 2)
        {
            throw std::runtime_error(
                fmt::format("Too many or too few matches ({}) to LHAPDF IDs.\nLHEPdfWeight description:\n "
                            "{}\nFirst 2 matches: {} and {}",
                            match_counter,
                            LHEPdfWeight_description,
                            buffer_ids[0],
                            buffer_ids[1]));
        }

        ids = std::make_pair(static_cast<unsigned int>(std::stoi(buffer_ids[0])),
                             static_cast<unsigned int>(std::stoi(buffer_ids[1])));
    }
    return ids;
}

inline double get_pdf_weight_from_LHAPDf(const TTree *events_tree)
{
    return 1.;
}

inline std::pair<std::string, int> get_pdfset_name_by_id(int &id)
{
    // Reference:
    // https://lhapdf.hepforge.org/group__index.html#gaa361996fe42aba7f8752a7be03166a47
    // Look up a PDF set name and member ID by the LHAPDF ID code
    // The set name and member ID are returned as an std::pair. If lookup fails, a pair ("", -1) is returned.
    auto pdf_set = LHAPDF::lookupPDF(id);

    if (pdf_set.second < 0)
    {
        throw std::runtime_error("Could not find PDF set for given id (" + std::to_string(id) + ").");
    }

    return pdf_set;
}

inline double get_pdf_weight_from_sample(const TTree *events_tree, const std::pair<int, int> &ids)
{
    return 1.;
}

// // TODO:
// // Add systematics
// inline double get_pdf_weight(TTree *events_tree)
// {
//     const auto ids = get_pdf_ids(events_tree);
//     if (ids)
//     {
//         return get_pdf_weight_from_sample(events_tree, ids.value());
//     }
//     return get_pdf_weight_from_LHAPDf(events_tree);
// }

} // namespace PDFAlphaSWeights

#endif // !PDF_ALPHA_S
