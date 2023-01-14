// #include <regex>

// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wunused-local-typedefs"
// #include "LHAPDF/LHAPDF.h"
// #pragma GCC diagnostic pop

// // References:
// //
// https://cms-pdmv.gitbook.io/project/mccontact/info-for-mc-production-for-ultra-legacy-campaigns-2016-2017-2018#recommendations-on-the-usage-of-pdfs-and-cpx-tunes

// namespace PDFAlphaSWeights
// {

// bool has_pdf_weight(const std::unique_ptr<TTree> &events_tree)
// {
//     if (events_tree->GetBranch("LHEPdfWeight"))
//     {
//         return true;
//     }
//     return false;
// }

// // will look for IDs in the LHEPdfWeight description string
// // if can not find then, will return a std::nullopt
// std::optional<std::pair<int, int>> get_pdf_ids(const std::unique_ptr<TTree> &events_tree)
// {
//     std::optional<std::pair<int, int>> ids = std::nullopt;
//     if (has_pdf_weight(events_tree))
//     {
//         std::string LHEPdfWeight_description = events_tree->GetBranch("LHEPdfWeight")->GetTitle();
//         std::regex expression("[0-9]*");

//         // default constructor = end-of-sequence:
//         std::regex_token_iterator<std::string::iterator> regex_end;
//         std::regex_token_iterator<std::string::iterator> regex_iter(LHEPdfWeight_description.begin(),
//                                                                     LHEPdfWeight_description.end(), expression);
//         auto match_counter = 0;
//         std::array<int, 2> buffer_ids = {-1, -1};
//         try
//         {
//             while (regex_iter != regex_end)
//             {
//                 buffer_ids.at(match_counter) = std::stoi(*regex_iter++);
//                 match_counter++;
//             }
//             if (match_counter != 2)
//             {
//                 throw std::runtime_error("Too many of too few matches (" + std::to_string(match_counter) + ") to
//                 LHAPDF IDs.");
//             }
//         }
//         catch (const std::out_of_range &e)
//         {
//             std::cerr << e.what() << std::endl;
//             std::cerr << "ERROR: When trying to match the LHEPdfWeight description the to declared PDF set IDs
//             (REGEX: "
//                          "\"[0-9]*\"), looks like there is more than 2 matches. \nLHEPdfWeight description:\n "
//                       << LHEPdfWeight_description << "\nFirst 2 matches: \n"
//                       << buffer_ids[0] << " and " << buffer_ids[1] << std::endl;
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << e.what() << std::endl;
//         }
//         ids = std::make_pair(buffer_ids[0], buffer_ids[1]);
//     }
//     return ids;
// }

// double get_pdf_weight_from_LHAPDf(const std::unique_ptr<TTree> &events_tree)
// {
//     return 1.;
// }

// std::pair<std::string, int> get_pdfset_name_by_id(int &id)
// {
//     // Reference:
//     // https://lhapdf.hepforge.org/group__index.html#gaa361996fe42aba7f8752a7be03166a47
//     // Look up a PDF set name and member ID by the LHAPDF ID code
//     // The set name and member ID are returned as an std::pair. If lookup fails, a pair ("", -1) is returned.
//     auto pdf_set = LHAPDF::lookupPDF(id);

//     if (pdf_set.second < 0)
//     {
//         throw std::runtime_error("Could not find PDF set for given id (" + std::to_string(id) + ").");
//     }

//     return pdf_set;
// }

// double get_pdf_weight_from_sample(const std::unique_ptr<TTree> &events_tree, const std::pair<int, int> &ids)
// {
//     return 1.;
// }

// // TODO:
// // Add systematics
// double get_pdf_weight(const std::unique_ptr<TTree> &events_tree)
// {
//     const auto ids = get_pdf_ids(events_tree);
//     if (ids)
//     {
//         return get_pdf_weight_from_sample(events_tree, ids.value());
//     }
//     return get_pdf_weight_from_LHAPDf(events_tree);
// }

// } // namespace PDFAlphaSWeights
