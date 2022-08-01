#ifndef NanoAODReader_hh
#define NanoAODReader_hh

#include <string>
#include <sstream>
#include <iostream>

#include "TLeaf.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"



  class NanoAODReader
  {
  public:
    NanoAODReader(TTree &tree);

    ~NanoAODReader();

    TTreeReader *getReader();

    std::vector<std::string> getListOfBranches();

    bool next();

    void printContent();

    void getTemplate(std::string &particle);

    std::string eraseSubString(std::string mainStr, const std::string &toErase);

    template <typename T>
    T getVal(std::string valueName)
    {
      return *(*(dynamic_cast<TTreeReaderValue<T> *>(fData[valueName].get())));
    }

    template <class T, class T2 = T>
    std::vector<T2> getVec(std::string vectorName)
    {
      auto array_temp_ = dynamic_cast<TTreeReaderArray<T> *>(fData[vectorName].get());
      return std::vector<T>(array_temp_->begin(), array_temp_->end());
    }

  private:
    TTreeReader fReader;                                                                // tree reader
    TObjArray fListOfLeaves;                                                            // list of available TLeaf's
    std::vector<std::string> fListOfBranches;                                           // branch names
    std::map<std::string, std::unique_ptr<ROOT::Internal::TTreeReaderValueBase>> fData; // data held by fTree
  };



template <>
inline std::vector<UInt_t> NanoAODReader::getVec<Bool_t>(std::string vectorName)
{
  auto array_temp_ = dynamic_cast<TTreeReaderArray<Bool_t> *>(fData[vectorName].get());
  return std::vector<UInt_t>(array_temp_->begin(), array_temp_->end());
}
// #include "NanoAODReader_imp.hh"

#endif
