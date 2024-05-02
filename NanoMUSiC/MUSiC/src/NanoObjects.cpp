#include "Math/Vector4Dfwd.h"
#include "NanoAODGenInfo.hpp"

namespace NanoAODGenInfo
{

GeneratorInfo::GeneratorInfo(const float &_binvar,
                             const float &_scalePDF,
                             const float &_weight,
                             const float &_x1,
                             const float &_x2,
                             const float &_xpdf1,
                             const float &_xpdf2,
                             const int &_id1,
                             const int &_id2)
    : binvar(_binvar),
      scalePDF(_scalePDF),
      weight(_weight),
      x1(_x1),
      x2(_x2),
      xpdf1(_xpdf1),
      xpdf2(_xpdf2),
      id1(_id1),
      id2(_id2)
{
}

LHEInfo::LHEInfo(const RVec<float> &_LHEPdfWeight,
                 const RVec<float> &_LHEScaleWeight,
                 const float &_originalXWGTUP,
                 const float &_HT,
                 const float &_HTIncoming)
    : nLHEPdfWeight(_LHEPdfWeight.size()),
      LHEPdfWeight(_LHEPdfWeight),
      nLHEScaleWeight(_LHEScaleWeight.size()),
      LHEScaleWeight(_LHEScaleWeight),
      originalXWGTUP(_originalXWGTUP),
      HT(_HT),
      HTIncoming(_HTIncoming)
{
}

GenParticles::GenParticles(const RVec<float> &_pt,
                           const RVec<float> &_eta,
                           const RVec<float> &_phi,
                           const RVec<float> &_mass,
                           const RVec<int> &_genPartIdxMother,
                           const RVec<int> &_pdgId,
                           const RVec<int> &_status,
                           const RVec<int> &_statusFlags)
    : nGenParticles(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi),
      mass(_mass),
      genPartIdxMother(_genPartIdxMother),
      pdgId(_pdgId),
      status(_status),
      statusFlags(_statusFlags)
{
}

LHEParticles::LHEParticles(const RVec<float> _pt,
                           const RVec<float> _eta,
                           const RVec<float> _phi,
                           const RVec<float> _mass,
                           const RVec<float> _incomingpz,
                           const RVec<int> _pdgId,
                           const RVec<int> _status)
    : nLHEParticles(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi),
      mass(_mass),
      incomingpz(_incomingpz),
      pdgId(_pdgId),
      status(_status)
{
}

GenJets::GenJets(const RVec<float> _pt, const RVec<float> _eta, const RVec<float> _phi)
    : size(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi)
{
}


} // namespace NanoAODGenInfo
