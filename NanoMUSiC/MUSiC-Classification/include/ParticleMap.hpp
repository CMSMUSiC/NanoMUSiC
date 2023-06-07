#ifndef PARTICLEMAP_HPP
#define PARTICLEMAP_HPP

#include <functional>
#include <string>
#include <unordered_map>

#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

class ParticleMap
{
    std::string name;
    std::string kind;

    std::unordered_map<std::string, RVec<Math::PtEtaPhiMVector>> m_map;

    // Scale Factor and error types:
    // scale_factor
    // scale_factor_error
    // scale_factor_error_syst
    // map<particle_name, map<scale factor or error, values as double>
    std::unordered_map<std::string, std::unordered_map<std::string, RVec<double>>> m_scale_factor_map;
    std::unordered_map<std::string, RVec<bool>> m_match_map;

    std::unordered_map<std::string, int> m_countMap;
    std::unordered_map<std::string, std::function<double(double)>> m_resolutionFuncMap;
    const std::unordered_map<std::string, std::function<double(Math::PtEtaPhiMVector)>> m_funcMap;

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
