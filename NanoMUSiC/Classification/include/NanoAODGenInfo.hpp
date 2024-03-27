#ifndef NanoAODGenInfo_H
#define NanoAODGenInfo_H

#include "ROOT/RVec.hxx"
#include <cstdlib>

using namespace ROOT;
using namespace ROOT::VecOps;
using namespace std::string_literals;

namespace NanoAODGenInfo
{

struct GeneratorInfo
{
    float binvar;
    float scalePDF;
    float weight;
    float x1;
    float x2;
    float xpdf1;
    float xpdf2;
    int id1;
    int id2;

    GeneratorInfo(const float &_binvar = 0.,
                  const float &_scalePDF = 0.,
                  const float &_weight = 1.,
                  const float &_x1 = 0.,
                  const float &_x2 = 0.,
                  const float &_xpdf1 = 0.,
                  const float &_xpdf2 = 0.,
                  const int &_id1 = 0,
                  const int &_id2 = 0);
};

struct LHEInfo
{
    std::size_t nLHEPdfWeight;
    RVec<float> LHEPdfWeight;
    std::size_t nLHEScaleWeight;
    RVec<float> LHEScaleWeight;
    float originalXWGTUP;
    float HT;
    float HTIncoming;

    LHEInfo(const RVec<float> &_LHEPdfWeight = {},
            const RVec<float> &_LHEScaleWeight = {},
            const float &_originalXWGTUP = 1.,
            const float &_HT = 0.,
            const float &_HTIncoming = 0.);
};

struct GenParticles
{
    std::size_t nGenParticles;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<float> mass;
    RVec<int> genPartIdxMother;
    RVec<int> pdgId;
    RVec<int> status;
    RVec<int> statusFlags;

    GenParticles(const RVec<float> &_pt = {},
                 const RVec<float> &_eta = {},
                 const RVec<float> &_phi = {},
                 const RVec<float> &_mass = {},
                 const RVec<int> &_genPartIdxMother = {},
                 const RVec<int> &_pdgId = {},
                 const RVec<int> &_status = {},
                 const RVec<int> &_statusFlags = {});
};

struct LHEParticles
{
    std::size_t nLHEParticles;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;
    RVec<float> mass;
    RVec<float> incomingpz;
    RVec<int> pdgId;
    RVec<int> status;

    LHEParticles(const RVec<float> _pt = {},
                 const RVec<float> _eta = {},
                 const RVec<float> _phi = {},
                 const RVec<float> _mass = {},
                 const RVec<float> _incomingpz = {},
                 const RVec<int> _pdgId = {},
                 const RVec<int> _status = {});
};

struct GenJets
{
    std::size_t size;
    RVec<float> pt;
    RVec<float> eta;
    RVec<float> phi;

    GenJets(const RVec<float> _pt = {}, const RVec<float> _eta = {}, const RVec<float> _phi = {});
};

} // namespace NanoAODGenInfo

#endif // NanoAODGenInfo_H
