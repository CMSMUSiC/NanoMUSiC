#ifndef PARTICLE_SPLITTING
#define PARTICLE_SPLITTING

#include <functional>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

namespace Splitting
{

// split one particle in two new particles according to splittingFunc
void split(std::map<std::string, std::vector<pxl::Particle *>> &map,
           const std::function<bool(pxl::Particle *)> &splittingFunc, const std::string &origName,
           const std::string &name1, const std::string &name2);
// merge two lists
void merge(std::map<std::string, std::vector<pxl::Particle *>> &map, const std::string &name1, const std::string &name2,
           const std::string &newName);
void merge(std::map<std::string, std::vector<pxl::Particle *>> &map, const std::vector<std::string> &names,
           const std::string &newName);

void splitBjets(std::map<std::string, std::vector<pxl::Particle *>> &map);
void splitGammaEndcap(std::map<std::string, std::vector<pxl::Particle *>> &map);
void splitEleEndcap(std::map<std::string, std::vector<pxl::Particle *>> &map);
void splitWfatjets(std::map<std::string, std::vector<pxl::Particle *>> &map);

void mergeWfatjets(std::map<std::string, std::vector<pxl::Particle *>> &map);
void mergeBjets(std::map<std::string, std::vector<pxl::Particle *>> &map);

void mergeZ(std::map<std::string, std::vector<pxl::Particle *>> &map, double deltaM, std::list<std::string> particles,
            std::string newName);
} // namespace Splitting

#endif
