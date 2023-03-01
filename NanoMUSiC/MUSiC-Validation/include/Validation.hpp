#ifndef VALIDATION
#define VALIDATION

#include <optional>
#include <sys/time.h>

#include "argh.h"

#include "fmt/format.h"

// ROOT Stuff
#include "Math/Vector4D.h"
#include "Math/VectorUtil.h"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "RtypesCore.h"
#include "TChain.h"
#include "TFile.h"
#include "TH1.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

#include "MUSiCTools.hpp"
#include "TOMLConfig.hpp"
#include "ZToLepLepX.hpp"
#include "emoji.hpp"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

// helper macros
#define ADD_VALUE_READER(VAR, TYPE) auto VAR = TTreeReaderValue<TYPE>(tree_reader, #VAR)
#define ADD_ARRAY_READER(VAR, TYPE) auto VAR = TTreeReaderArray<TYPE>(tree_reader, #VAR)

template <typename T>
auto unwrap(TTreeReaderValue<T> &value) -> T
{
    return *value;
}

template <typename T>
auto unwrap(TTreeReaderArray<T> &array) -> RVec<T>
{
    return RVec<T>(static_cast<T *>(array.GetAddress()), array.GetSize());
}

inline auto PrintProcessInfo() -> void
{
    auto info = ProcInfo_t();
    gSystem->GetProcInfo(&info);
    std::cout.precision(1);
    std::cout << std::fixed;
    std::cout << "-------------" << std::endl;
    std::cout << "Process info:" << std::endl;
    std::cout << "-------------" << std::endl;
    std::cout << "CPU time elapsed: " << info.fCpuUser << " s" << std::endl;
    std::cout << "Sys time elapsed: " << info.fCpuSys << " s" << std::endl;
    std::cout << "Resident memory:  " << info.fMemResident / 1024. << " MB" << std::endl;
    std::cout << "Virtual memory:   " << info.fMemVirtual / 1024. << " MB" << std::endl;
}

inline auto getCpuTime() -> double
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return ((double)tv.tv_sec + (double)tv.tv_usec / 1000000.0);
}

#endif // VALIDATION