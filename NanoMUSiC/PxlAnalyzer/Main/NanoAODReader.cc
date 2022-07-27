

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
NanoAODReader::NanoAODReader(TTree *tree_ptr)
{
  fTree = tree_ptr;
  fReader = new TTreeReader(fTree);

  // fill map with values readers
  for (auto const &leaf : *(fTree->GetListOfLeaves()))
  {
    auto leaf_temp = dynamic_cast<TLeaf *>(leaf);
    std::string leaf_name = (std::string)(leaf_temp->GetName());
    std::string leaf_type = (std::string)(leaf_temp->GetTypeName());

    fListOfBranches.push_back(leaf_name);

    void *value_reader = NULL;

    // check if data is array or single value
    if (leaf_temp->GetLeafCount() != nullptr || leaf_temp->GetLenStatic() > 1)
    {
      if (leaf_type == "Char_t")
      {
        value_reader = new TTreeReaderArray<Char_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "UChar_t")
      {
        value_reader = new TTreeReaderArray<UChar_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Short_t")
      {
        value_reader = new TTreeReaderArray<Short_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "UShort_t")
      {
        value_reader = new TTreeReaderArray<UShort_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Int_t")
      {
        value_reader = new TTreeReaderArray<Int_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "UInt_t")
      {
        value_reader = new TTreeReaderArray<UInt_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Seek_t")
      {
        value_reader = new TTreeReaderArray<Seek_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Long_t")
      {
        value_reader = new TTreeReaderArray<Long_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "ULong_t")
      {
        value_reader = new TTreeReaderArray<ULong_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Float_t")
      {
        value_reader = new TTreeReaderArray<Float_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Float16_t")
      {
        value_reader = new TTreeReaderArray<Float16_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Double_t")
      {
        value_reader = new TTreeReaderArray<Double_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Double32_t")
      {
        value_reader = new TTreeReaderArray<Double32_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "LongDouble_t")
      {
        value_reader = new TTreeReaderArray<LongDouble_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Text_t")
      {
        value_reader = new TTreeReaderArray<Text_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Bool_t")
      {
        value_reader = new TTreeReaderArray<Bool_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Byte_t")
      {
        value_reader = new TTreeReaderArray<Byte_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Version_t")
      {
        value_reader = new TTreeReaderArray<Version_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Option_t")
      {
        value_reader = new TTreeReaderArray<Option_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Ssiz_t")
      {
        value_reader = new TTreeReaderArray<Ssiz_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Real_t")
      {
        value_reader = new TTreeReaderArray<Real_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Long64_t")
      {
        value_reader = new TTreeReaderArray<Long64_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "ULong64_t")
      {
        value_reader = new TTreeReaderArray<ULong64_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Axis_t")
      {
        value_reader = new TTreeReaderArray<Axis_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Stat_t")
      {
        value_reader = new TTreeReaderArray<Stat_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Font_t")
      {
        value_reader = new TTreeReaderArray<Font_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Style_t")
      {
        value_reader = new TTreeReaderArray<Style_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Marker_t")
      {
        value_reader = new TTreeReaderArray<Marker_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Width_t")
      {
        value_reader = new TTreeReaderArray<Width_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Color_t")
      {
        value_reader = new TTreeReaderArray<Color_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "SCoord_t")
      {
        value_reader = new TTreeReaderArray<SCoord_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Coord_t")
      {
        value_reader = new TTreeReaderArray<Coord_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Angle_t")
      {
        value_reader = new TTreeReaderArray<Angle_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Size_t")
      {
        value_reader = new TTreeReaderArray<Size_t>(*fReader, leaf_name.c_str());
      }
    }
    else
    {
      if (leaf_type == "Char_t")
      {
        value_reader = new TTreeReaderValue<Char_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "UChar_t")
      {
        value_reader = new TTreeReaderValue<UChar_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Short_t")
      {
        value_reader = new TTreeReaderValue<Short_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "UShort_t")
      {
        value_reader = new TTreeReaderValue<UShort_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Int_t")
      {
        value_reader = new TTreeReaderValue<Int_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "UInt_t")
      {
        value_reader = new TTreeReaderValue<UInt_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Seek_t")
      {
        value_reader = new TTreeReaderValue<Seek_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Long_t")
      {
        value_reader = new TTreeReaderValue<Long_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "ULong_t")
      {
        value_reader = new TTreeReaderValue<ULong_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Float_t")
      {
        value_reader = new TTreeReaderValue<Float_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Float16_t")
      {
        value_reader = new TTreeReaderValue<Float16_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Double_t")
      {
        value_reader = new TTreeReaderValue<Double_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Double32_t")
      {
        value_reader = new TTreeReaderValue<Double32_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "LongDouble_t")
      {
        value_reader = new TTreeReaderValue<LongDouble_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Text_t")
      {
        value_reader = new TTreeReaderValue<Text_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Bool_t")
      {
        value_reader = new TTreeReaderValue<Bool_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Byte_t")
      {
        value_reader = new TTreeReaderValue<Byte_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Version_t")
      {
        value_reader = new TTreeReaderValue<Version_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Option_t")
      {
        value_reader = new TTreeReaderValue<Option_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Ssiz_t")
      {
        value_reader = new TTreeReaderValue<Ssiz_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Real_t")
      {
        value_reader = new TTreeReaderValue<Real_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Long64_t")
      {
        value_reader = new TTreeReaderValue<Long64_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "ULong64_t")
      {
        value_reader = new TTreeReaderValue<ULong64_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Axis_t")
      {
        value_reader = new TTreeReaderValue<Axis_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Stat_t")
      {
        value_reader = new TTreeReaderValue<Stat_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Font_t")
      {
        value_reader = new TTreeReaderValue<Font_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Style_t")
      {
        value_reader = new TTreeReaderValue<Style_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Marker_t")
      {
        value_reader = new TTreeReaderValue<Marker_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Width_t")
      {
        value_reader = new TTreeReaderValue<Width_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Color_t")
      {
        value_reader = new TTreeReaderValue<Color_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "SCoord_t")
      {
        value_reader = new TTreeReaderValue<SCoord_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Coord_t")
      {
        value_reader = new TTreeReaderValue<Coord_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Angle_t")
      {
        value_reader = new TTreeReaderValue<Angle_t>(*fReader, leaf_name.c_str());
      }
      if (leaf_type == "Size_t")
      {
        value_reader = new TTreeReaderValue<Size_t>(*fReader, leaf_name.c_str());
      }
    }

    fData[leaf_name] = value_reader;
  }
}

NanoAODReader::~NanoAODReader() {}

TTreeReader *NanoAODReader::getReader()
{
  return fReader;
}

std::vector<std::string> NanoAODReader::getListOfBranches()
{
  return fListOfBranches;
}

bool NanoAODReader::next()
{
  return fReader->Next();
}

void NanoAODReader::printContent()
{
  std::cout << "\n\n\n\n\n"
            << std::endl;
  std::cout << "NanoAOD File Content:" << std::endl;

  // fill map with values readers
  for (auto const &leaf : *(fTree->GetListOfLeaves()))
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
      std::cout << leaf_name << std::string(length_diff+1, ' ') << " - "
                << "Value < " << leaf_type << " >" << std::endl;
    }
  }

  std::cout << "\n\n\n\n\n"
            << std::endl;
}
