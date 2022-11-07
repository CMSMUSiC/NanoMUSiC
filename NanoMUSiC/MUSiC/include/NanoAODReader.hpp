#ifndef NanoAODReader_hh
#define NanoAODReader_hh

#include <iostream>
#include <sstream>
#include <string>

#include "TLeaf.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

void simple_replace(std::string &str_in, const std::string &str_old, const std::string &str_new);

// Load and read NanoAOD files, regardsless of its implementation version.
class NanoAODReader
{
  public:
    NanoAODReader(TTree &tree);

    ~NanoAODReader();

    TTreeReader *getReader();

    std::vector<std::string> getListOfBranches();

    bool next();

    // Print NanoAOD file content.
    void printContent();

    void getTemplate(std::string &particle);

    std::string eraseSubString(std::string mainStr, const std::string &toErase);

    bool checkIfBranchExists(std::string branch_name);

    std::string GetTypeName(const std::type_info &ti);

    template <typename T>
    T getVal(std::string valueName)
    {
        // check for branch name
        try
        {
            checkIfBranchExists(valueName);
        }
        catch (std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        // check for type matching
        try
        {
            auto type_temp_ = fType[valueName];
            simple_replace(type_temp_, "Char_t", "char");
            simple_replace(type_temp_, "UChar_t", "unsigned char");
            simple_replace(type_temp_, "Short_t", "short");
            simple_replace(type_temp_, "UShort_t", "unsigned short");
            simple_replace(type_temp_, "Int_t", "int");
            simple_replace(type_temp_, "UInt_t", "unsigned int");
            simple_replace(type_temp_, "Seek_t", "int");
            simple_replace(type_temp_, "Long_t", "long");
            simple_replace(type_temp_, "ULong_t", "unsigned long");
            simple_replace(type_temp_, "Float_t", "float");
            simple_replace(type_temp_, "Float16_t", "float");
            simple_replace(type_temp_, "Double_t", "double");
            simple_replace(type_temp_, "Double32_t", "double");
            simple_replace(type_temp_, "LongDouble_t", "long double");
            simple_replace(type_temp_, "Text_t", "char");
            simple_replace(type_temp_, "Bool_t", "bool");
            simple_replace(type_temp_, "Byte_t", "unsigned char");
            simple_replace(type_temp_, "Version_t", "short");
            simple_replace(type_temp_, "Option_t", "char");
            simple_replace(type_temp_, "Ssiz_t", "int");
            simple_replace(type_temp_, "Real_t", "float");
            simple_replace(type_temp_, "Long64_t", "long long");
            simple_replace(type_temp_, "ULong64_t", "unsigned long long");
            simple_replace(type_temp_, "Axis_t", "double");
            simple_replace(type_temp_, "Stat_t", "double");
            simple_replace(type_temp_, "Font_t", "short");
            simple_replace(type_temp_, "Style_t", "short");
            simple_replace(type_temp_, "Marker_t", "short");
            simple_replace(type_temp_, "Width_t", "short");
            simple_replace(type_temp_, "Color_t", "short");
            simple_replace(type_temp_, "SCoord_t", "short");
            simple_replace(type_temp_, "Coord_t", "double");
            simple_replace(type_temp_, "Angle_t", "float");
            simple_replace(type_temp_, "Size_t", "float");

            if (GetTypeName(typeid(T)) != type_temp_)
            {

                throw std::runtime_error(
                    "--> ERROR: Trying to read branch \"" + valueName + "\" as \"" + GetTypeName(typeid(T)) +
                    "\" is not the same type defiend for this branch. Expected type is: \"" + fType[valueName] + "\".");
            }
        }
        catch (std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        return *(*(dynamic_cast<TTreeReaderValue<T> *>(fData[valueName].get())));
    }

    template <typename T, typename T2 = T>
    std::vector<T2> getVec(std::string vectorName)
    {
        // check for branch name
        try
        {
            checkIfBranchExists(vectorName);
        }
        catch (std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        // check for type matching
        try
        {
            auto type_temp_ = fType[vectorName];

            simple_replace(type_temp_, "Char_t", "char");
            simple_replace(type_temp_, "UChar_t", "unsigned char");
            simple_replace(type_temp_, "Short_t", "short");
            simple_replace(type_temp_, "UShort_t", "unsigned short");
            simple_replace(type_temp_, "Int_t", "int");
            simple_replace(type_temp_, "UInt_t", "unsigned int");
            simple_replace(type_temp_, "Seek_t", "int");
            simple_replace(type_temp_, "Long_t", "long");
            simple_replace(type_temp_, "ULong_t", "unsigned long");
            simple_replace(type_temp_, "Float_t", "float");
            simple_replace(type_temp_, "Float16_t", "float");
            simple_replace(type_temp_, "Double_t", "double");
            simple_replace(type_temp_, "Double32_t", "double");
            simple_replace(type_temp_, "LongDouble_t", "long double");
            simple_replace(type_temp_, "Text_t", "char");
            simple_replace(type_temp_, "Bool_t", "bool");
            simple_replace(type_temp_, "Byte_t", "unsigned char");
            simple_replace(type_temp_, "Version_t", "short");
            simple_replace(type_temp_, "Option_t", "char");
            simple_replace(type_temp_, "Ssiz_t", "int");
            simple_replace(type_temp_, "Real_t", "float");
            simple_replace(type_temp_, "Long64_t", "long long");
            simple_replace(type_temp_, "ULong64_t", "unsigned long long");
            simple_replace(type_temp_, "Axis_t", "double");
            simple_replace(type_temp_, "Stat_t", "double");
            simple_replace(type_temp_, "Font_t", "short");
            simple_replace(type_temp_, "Style_t", "short");
            simple_replace(type_temp_, "Marker_t", "short");
            simple_replace(type_temp_, "Width_t", "short");
            simple_replace(type_temp_, "Color_t", "short");
            simple_replace(type_temp_, "SCoord_t", "short");
            simple_replace(type_temp_, "Coord_t", "double");
            simple_replace(type_temp_, "Angle_t", "float");
            simple_replace(type_temp_, "Size_t", "float");

            if (GetTypeName(typeid(T)) != type_temp_)
            {
                throw std::runtime_error("--> ERROR: Trying to read branch \"" + vectorName + "\" as \"" +
                                         GetTypeName(typeid(T)) +
                                         "\" is not the same type defiend for this branch. Expected type is: \"" +
                                         fType[vectorName] + "\".");
            }
        }
        catch (std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        auto array_temp_ = dynamic_cast<TTreeReaderArray<T> *>(fData[vectorName].get());
        return std::vector<T>(array_temp_->begin(), array_temp_->end());
    }

    // deals with std:vector<bool> (bleh!!!!)
    // will return a std:vector<UInt_t>
    std::vector<UInt_t> getVecOfBools(std::string vectorName)
    {
        // check for branch name
        try
        {
            checkIfBranchExists(vectorName);
        }
        catch (std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        // check for type matching
        try
        {
            auto type_temp_ = fType[vectorName];
            simple_replace(type_temp_, "Char_t", "char");
            simple_replace(type_temp_, "UChar_t", "unsigned char");
            simple_replace(type_temp_, "Short_t", "short");
            simple_replace(type_temp_, "UShort_t", "unsigned short");
            simple_replace(type_temp_, "Int_t", "int");
            simple_replace(type_temp_, "UInt_t", "unsigned int");
            simple_replace(type_temp_, "Seek_t", "int");
            simple_replace(type_temp_, "Long_t", "long");
            simple_replace(type_temp_, "ULong_t", "unsigned long");
            simple_replace(type_temp_, "Float_t", "float");
            simple_replace(type_temp_, "Float16_t", "float");
            simple_replace(type_temp_, "Double_t", "double");
            simple_replace(type_temp_, "Double32_t", "double");
            simple_replace(type_temp_, "LongDouble_t", "long double");
            simple_replace(type_temp_, "Text_t", "char");
            simple_replace(type_temp_, "Bool_t", "bool");
            simple_replace(type_temp_, "Byte_t", "unsigned char");
            simple_replace(type_temp_, "Version_t", "short");
            simple_replace(type_temp_, "Option_t", "char");
            simple_replace(type_temp_, "Ssiz_t", "int");
            simple_replace(type_temp_, "Real_t", "float");
            simple_replace(type_temp_, "Long64_t", "long long");
            simple_replace(type_temp_, "ULong64_t", "unsigned long long");
            simple_replace(type_temp_, "Axis_t", "double");
            simple_replace(type_temp_, "Stat_t", "double");
            simple_replace(type_temp_, "Font_t", "short");
            simple_replace(type_temp_, "Style_t", "short");
            simple_replace(type_temp_, "Marker_t", "short");
            simple_replace(type_temp_, "Width_t", "short");
            simple_replace(type_temp_, "Color_t", "short");
            simple_replace(type_temp_, "SCoord_t", "short");
            simple_replace(type_temp_, "Coord_t", "double");
            simple_replace(type_temp_, "Angle_t", "float");
            simple_replace(type_temp_, "Size_t", "float");

            if (GetTypeName(typeid(bool)) != type_temp_)
            {
                throw std::runtime_error("--> ERROR: Trying to read branch \"" + vectorName + "\" as \"" +
                                         GetTypeName(typeid(Bool_t)) +
                                         "\" is not the same type defined for this branch. Expected type is: \"" +
                                         fType[vectorName] + "\".");
            }
        }
        catch (std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
            exit(1);
        }

        auto array_temp_ = dynamic_cast<TTreeReaderArray<Bool_t> *>(fData[vectorName].get());
        std::vector<UInt_t> _buffer;
        for (const auto &elem : *array_temp_)
        {
            _buffer.emplace_back(elem);
        }
        return _buffer;
    }

  private:
    TTreeReader fReader;                                                                // tree reader
    TObjArray fListOfLeaves;                                                            // list of available TLeaf's
    std::vector<std::string> fListOfBranches;                                           // branch names
    std::map<std::string, std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>> fData; // data held by fTree
    std::map<std::string, std::string> fType;                                           // types held by fTree
};

// // deals with std:vector<bool> (bleh!!!!)
// // will return a std:vector<UInt_t>
// template <>
// inline std::vector<UInt_t> NanoAODReader::getVec<Bool_t>(std::string vectorName)
// {
//     // check for branch name
//     try
//     {
//         checkIfBranchExists(vectorName);
//     }
//     catch (std::runtime_error &e)
//     {
//         std::cerr << e.what() << std::endl;
//         exit(1);
//     }

//     // check for type matching
//     try
//     {
//         auto type_temp_ = fType[vectorName];
//         simple_replace(type_temp_, "Char_t", "char");
//         simple_replace(type_temp_, "UChar_t", "unsigned char");
//         simple_replace(type_temp_, "Short_t", "short");
//         simple_replace(type_temp_, "UShort_t", "unsigned short");
//         simple_replace(type_temp_, "Int_t", "int");
//         simple_replace(type_temp_, "UInt_t", "unsigned int");
//         simple_replace(type_temp_, "Seek_t", "int");
//         simple_replace(type_temp_, "Long_t", "long");
//         simple_replace(type_temp_, "ULong_t", "unsigned long");
//         simple_replace(type_temp_, "Float_t", "float");
//         simple_replace(type_temp_, "Float16_t", "float");
//         simple_replace(type_temp_, "Double_t", "double");
//         simple_replace(type_temp_, "Double32_t", "double");
//         simple_replace(type_temp_, "LongDouble_t", "long double");
//         simple_replace(type_temp_, "Text_t", "char");
//         simple_replace(type_temp_, "Bool_t", "bool");
//         simple_replace(type_temp_, "Byte_t", "unsigned char");
//         simple_replace(type_temp_, "Version_t", "short");
//         simple_replace(type_temp_, "Option_t", "char");
//         simple_replace(type_temp_, "Ssiz_t", "int");
//         simple_replace(type_temp_, "Real_t", "float");
//         simple_replace(type_temp_, "Long64_t", "long long");
//         simple_replace(type_temp_, "ULong64_t", "unsigned long long");
//         simple_replace(type_temp_, "Axis_t", "double");
//         simple_replace(type_temp_, "Stat_t", "double");
//         simple_replace(type_temp_, "Font_t", "short");
//         simple_replace(type_temp_, "Style_t", "short");
//         simple_replace(type_temp_, "Marker_t", "short");
//         simple_replace(type_temp_, "Width_t", "short");
//         simple_replace(type_temp_, "Color_t", "short");
//         simple_replace(type_temp_, "SCoord_t", "short");
//         simple_replace(type_temp_, "Coord_t", "double");
//         simple_replace(type_temp_, "Angle_t", "float");
//         simple_replace(type_temp_, "Size_t", "float");

//         if (GetTypeName(typeid(bool)) != type_temp_)
//         {
//             throw std::runtime_error(
//                 "--> ERROR: Trying to read branch \"" + vectorName + "\" as \"" + GetTypeName(typeid(Bool_t)) +
//                 "\" is not the same type defined for this branch. Expected type is: \"" + fType[vectorName] + "\".");
//         }
//     }
//     catch (std::runtime_error &e)
//     {
//         std::cerr << e.what() << std::endl;
//         exit(1);
//     }

//     auto array_temp_ = dynamic_cast<TTreeReaderArray<Bool_t> *>(fData[vectorName].get());
//     std::cout << "Passei por aaqui!!" << std::endl;
//     return std::vector<UInt_t>(array_temp_->begin(), array_temp_->end());
// }

#endif
