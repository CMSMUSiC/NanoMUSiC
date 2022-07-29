

#include "TLeaf.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "ROOT/RVec.hxx"

#include "NanoAODReader.hh"

#include <string>
#include <sstream>
#include <iostream>

// Constructor
NanoAODReader::NanoAODReader(TTree &tree) : fReader(TTreeReader(&tree)),
                                            fListOfLeaves(*(tree.GetListOfLeaves()))
{
  // fill map with values readers
  for (auto const &leaf : fListOfLeaves)
  {
    auto leaf_temp = dynamic_cast<TLeaf *>(leaf);
    std::string leaf_name = (std::string)(leaf_temp->GetName());
    std::string leaf_type = (std::string)(leaf_temp->GetTypeName());

    fListOfBranches.push_back(leaf_name);

    // check if data is array or single value
    if (leaf_temp->GetLeafCount() != nullptr || leaf_temp->GetLenStatic() > 1)
    {
      if (leaf_type == "Char_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Char_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "UChar_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<UChar_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Short_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Short_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "UShort_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<UShort_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Int_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Int_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "UInt_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<UInt_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Seek_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Seek_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Long_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Long_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "ULong_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<ULong_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Float_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Float_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Float16_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Float16_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Double_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Double_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Double32_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Double32_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "LongDouble_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<LongDouble_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Text_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Text_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Bool_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Bool_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Byte_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Byte_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Version_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Version_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Option_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Option_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Ssiz_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Ssiz_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Real_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Real_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Long64_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Long64_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "ULong64_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<ULong64_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Axis_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Axis_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Stat_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Stat_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Font_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Font_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Style_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Style_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Marker_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Marker_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Width_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Width_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Color_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Color_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "SCoord_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<SCoord_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Coord_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Coord_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Angle_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Angle_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Size_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderArray<Size_t>(fReader, leaf_name.c_str())));
      }
    }
    else
    {
      if (leaf_type == "Char_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Char_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "UChar_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<UChar_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Short_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Short_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "UShort_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<UShort_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Int_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Int_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "UInt_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<UInt_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Seek_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Seek_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Long_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Long_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "ULong_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<ULong_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Float_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Float_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Float16_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Float16_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Double_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Double_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Double32_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Double32_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "LongDouble_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<LongDouble_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Text_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Text_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Bool_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Bool_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Byte_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Byte_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Version_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Version_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Option_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Option_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Ssiz_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Ssiz_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Real_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Real_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Long64_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Long64_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "ULong64_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<ULong64_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Axis_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Axis_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Stat_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Stat_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Font_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Font_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Style_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Style_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Marker_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Marker_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Width_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Width_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Color_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Color_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "SCoord_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<SCoord_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Coord_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Coord_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Angle_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Angle_t>(fReader, leaf_name.c_str())));
      }
      if (leaf_type == "Size_t")
      {
        fData[leaf_name] = std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>(dynamic_cast<ROOT::Internal::TTreeReaderValueBase *>(new TTreeReaderValue<Size_t>(fReader, leaf_name.c_str())));
      }
    }
  }
}

NanoAODReader::~NanoAODReader()
{
}

TTreeReader *NanoAODReader::getReader()
{
  return &fReader;
}

std::vector<std::string> NanoAODReader::getListOfBranches()
{
  return fListOfBranches;
}

bool NanoAODReader::next()
{
  return fReader.Next();
}

void NanoAODReader::printContent()
{
  std::cout << "\n\n\n\n\n"
            << std::endl;
  std::cout << "NanoAOD File Content:" << std::endl;

  // fill map with values readers
  for (auto const &leaf : fListOfLeaves)
  {
    auto leaf_temp = dynamic_cast<TLeaf *>(leaf);
    std::string leaf_name = (std::string)(leaf_temp->GetName());
    std::string leaf_type = (std::string)(leaf_temp->GetTypeName());

    auto longest_leaf_name = std::max_element(fListOfBranches.begin(), fListOfBranches.end(),
                                              [](const auto &a, const auto &b)
                                              {
                                                return a.size() < b.size();
                                              });
    int length_diff = (*longest_leaf_name).size() - leaf_name.size();

    std::cout << std::string((*longest_leaf_name).size() + 25, '-') << std::endl;

    // check if data is array or single value
    if (leaf_temp->GetLeafCount() != nullptr || leaf_temp->GetLenStatic() > 1)
    {
      std::cout << leaf_name << std::string(length_diff, ' ') << " - "
                << "Vector < " << leaf_type << " >" << std::endl;
    }
    else
    {
      std::cout << leaf_name << std::string(length_diff, ' ') << " - "
                << "Value < " << leaf_type << " >" << std::endl;
    }
  }

  std::cout << "\n\n\n\n\n"
            << std::endl;
}
