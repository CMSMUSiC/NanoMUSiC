#ifndef TriggerStream_hh
#define TriggerStream_hh

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core.hpp"
#include "Pxl/Pxl/interface/pxl/hep.hpp"

using namespace std;

class TriggerStream
{
    map<string, int> s_unique_trigs;

  public:
    void addEntry(const pxl::Event *const event);
    void writeUniqueTriggers();
};

#endif /*TriggerStream_hh*/
