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
#include "ROOT/RVec.hxx"

using namespace ROOT::VecOps;
class NanoAODReader
{
public:
  NanoAODReader(TTree& tree );

  ~NanoAODReader();

  TTreeReader *getReader();

  std::vector<std::string> getListOfBranches();

  bool next();

  void printContent();

  template<typename T>
  T getVal(std::string valueName);

  template<typename T>
  RVec<T> getRVec(std::string vectorName);

private:
  TTreeReader fReader; // the tree reader
  TObjArray  fListOfLeaves;
  std::vector<std::string> fListOfBranches;

  std::map< std::string, std::unique_ptr<ROOT::Internal::TTreeReaderValueBase> > fData; // data  held by fTree
};

#include "NanoAODReader_imp.hh"


#endif
