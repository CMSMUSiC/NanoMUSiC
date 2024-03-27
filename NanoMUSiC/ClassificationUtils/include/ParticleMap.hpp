#ifndef PARTICLEMAP_HPP
#define PARTICLEMAP_HPP

#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>

#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

// Most probably this will be necessary, but I am trying to postpone as much as possible it implementation.
// Arrays of Structs are the source of all evil ...
// class Particle
// {
//   private:
//     Math::PtEtaPhiMVector m_fourvector;
//     double m_scale_factor;
//     bool m_has_match;

//   public:
//     Particle(const Math::PtEtaPhiMVector &fourvector, double scale_factor, bool has_match)
//         : m_fourvector(fourvector),
//           m_scale_factor(scale_factor),
//           m_has_match(has_match)
//     {
//     }

//     auto get_fourvector() -> Math::PtEtaPhiMVector const
//     {
//         return m_fourvector;
//     }

//     auto get_scale_factor() -> double const
//     {
//         return m_scale_factor;
//     }

//     auto has_match() -> bool const
//     {
//         return m_has_match;
//     }
// };

class ParticleMap
{
  public:
    using ParticleMap_t = std::unordered_map<std::string, RVec<Math::PtEtaPhiMVector>>;
    using ScaleFactorMap_t = std::unordered_map<std::string, std::unordered_map<std::string, RVec<double>>>;
    using MatchMap_t = std::unordered_map<std::string, RVec<bool>>;
    using CountMap_t = std::unordered_map<std::string, int>;
    using ResolutionFuncMap_t = std::unordered_map<std::string, std::function<double(double)>>;
    using FuncMap_t = std::unordered_map<std::string, std::function<double(Math::PtEtaPhiMVector)>>;

  private:
    std::string name;
    std::string kind;

    ParticleMap_t m_map;

    // Scale Factor and error types:
    // scale_factor
    // scale_factor_error
    // scale_factor_error_syst
    // map<particle_name, map<scale factor or error, values as double>
    ScaleFactorMap_t m_scale_factor_map;
    MatchMap_t m_match_map;

    CountMap_t m_countMap;

    ResolutionFuncMap_t m_resolutionFuncMap;

    const FuncMap_t m_funcMap;

    double getApproximateResolution(const std::unordered_map<std::string, int> &countMap,
                                    double sumpt,
                                    double const fudge = 1) const;
    double getApproximateResolutionMET(const std::unordered_map<std::string, int> &countMap,
                                       double sumpt,
                                       double const fudge = 1) const;
    double getSumPx(const std::unordered_map<std::string, int> &countMap) const;
    double getSumPy(const std::unordered_map<std::string, int> &countMap) const;
    double getSumEt(const std::unordered_map<std::string, int> &countMap) const;

    // not needed (?)
    // void split(std::vector<Math::PtEtaPhiMVector>> particles,
    //            std::function<bool(pxl::Particle *)> splittingFunc,
    //            std::string name1,
    //            std::string name2);
    double getKinematicVariable(const std::string var, const std::unordered_map<std::string, int> &countMap) const;
    double callResolutionFunction(const std::string &name, const double &value) const;
    double getScaleFactorError(const std::string error_type,
                               const std::unordered_map<std::string, int> &countMap) const;

  public:
    ParticleMap(const std::unordered_map<std::string, RVec<Math::PtEtaPhiMVector>> &particleMap,
                const std::unordered_map<std::string, std::unordered_map<std::string, RVec<double>>> &ScaleFactorMap,
                const std::unordered_map<std::string, RVec<bool>> &matchMap);

    static auto make_empty_particle_maps() -> std::tuple<ParticleMap_t, ScaleFactorMap_t, MatchMap_t>;

    auto erase(const std::string &field) -> void;

    const RVec<Math::PtEtaPhiMVector> &getParticleVector(std::string &name) const;
    std::unordered_map<std::string, int> getCountMap() const;
    // int getLeptonCharge(const std::unordered_map<std::string, int> &countMap) const;
    double getSumPt(const std::unordered_map<std::string, int> &countMap) const;
    double getMass(const std::unordered_map<std::string, int> &countMap) const;
    double getTransverseMass(const std::unordered_map<std::string, int> &countMap) const;
    double getInvMass(const std::unordered_map<std::string, int> &countMap) const;
    double getMET(const std::unordered_map<std::string, int> &countMap) const;
    double getScaleFactor(const std::unordered_map<std::string, int> &countMap) const;
    double getScaleFactorStatError(const std::unordered_map<std::string, int> &countMap) const;
    double getScaleFactorSystError(const std::unordered_map<std::string, int> &countMap) const;
    std::unordered_map<std::string, int> getFakeMap(const std::unordered_map<std::string, int> &countMap) const;

    // not needed (?)
    //  std::unordered_map<std::string, int> getChargeFakeMap(const std::unordered_map<std::string, int> &countMap)
    //  const;

    std::vector<double> getBinLimits(std::string distribution,
                                     std::unordered_map<std::string, int> &countMap,
                                     double min,
                                     double max,
                                     double step_size,
                                     double const fudge = 1) const;
    double getRealResolution(const std::unordered_map<std::string, int> &countMap) const;
    double getApproximateResolution(const std::unordered_map<std::string, int> &countMap, double const fudge = 1) const;
};

#endif
