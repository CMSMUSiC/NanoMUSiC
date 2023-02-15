#include "NanoObjects.hpp"

namespace LorentzVectorHelper
{

auto mass(const float &pt1,
          const float &eta1,
          const float &phi1,
          const float &mass1,
          const float &pt2,
          const float &eta2,
          const float &phi2,
          const float &mass2) -> float
{
    // Conversion from (pt, eta, phi, mass) to (x, y, z, e) coordinate system
    const auto x1 = pt1 * std::cos(phi1);
    const auto y1 = pt1 * std::sin(phi1);
    const auto z1 = pt1 * std::sinh(eta1);
    const auto e1 = std::sqrt(x1 * x1 + y1 * y1 + z1 * z1 + mass1 * mass1);

    const auto x2 = pt2 * std::cos(phi2);
    const auto y2 = pt2 * std::sin(phi2);
    const auto z2 = pt2 * std::sinh(eta2);
    const auto e2 = std::sqrt(x2 * x2 + y2 * y2 + z2 * z2 + mass2 * mass2);

    // Addition of particle four-vector elements
    const auto e = e1 + e2;
    const auto x = x1 + x2;
    const auto y = y1 + y2;
    const auto z = z1 + z2;

    return std::sqrt(e * e - x * x - y * y - z * z);
}

auto pt(const float &pt1, const float &phi1, const float &pt2, const float &phi2) -> float
{
    const auto x1 = pt1 * std::cos(phi1);
    const auto y1 = pt1 * std::sin(phi1);

    const auto x2 = pt2 * std::cos(phi2);
    const auto y2 = pt2 * std::sin(phi2);

    // Addition of particle 2d-vector components
    const auto x = x1 + x2;
    const auto y = y1 + y2;

    return std::sqrt(x * x + y * y);
}

} // namespace LorentzVectorHelper

namespace NanoObjects
{
EventInfo::EventInfo(const UInt_t &_run,
                     const UInt_t &_lumi,
                     const ULong64_t &_event,
                     const float &_Pileup_nTrueInt,
                     const float &_genWeight,
                     const float &_L1PreFiringWeight_Nom,
                     const float &_L1PreFiringWeight_Up,
                     const float &_L1PreFiringWeight_Dn,
                     const int &_PV_npvsGood,
                     const bool &_Flag_goodVertices,
                     const bool &_Flag_globalSuperTightHalo2016Filter,
                     const bool &_Flag_HBHENoiseFilter,
                     const bool &_Flag_HBHENoiseIsoFilter,
                     const bool &_Flag_EcalDeadCellTriggerPrimitiveFilter,
                     const bool &_Flag_BadPFMuonFilter,
                     const bool &_Flag_BadPFMuonDzFilter,
                     const bool &_Flag_eeBadScFilter,
                     const bool &_Flag_ecalBadCalibFilter,
                     const float &_rho,
                     const bool &_HLT_IsoMu27,
                     const bool &_HLT_IsoMu24,
                     const bool &_HLT_IsoTkMu24,
                     const bool &_HLT_Mu50,
                     const bool &_HLT_TkMu50,
                     const bool &_HLT_TkMu100,
                     const bool &_HLT_OldMu100,
                     const bool &_HLT_Ele27_WPTight_Gsf,
                     const bool &_HLT_Ele35_WPTight_Gsf,
                     const bool &_HLT_Ele32_WPTight_Gsf,
                     const bool &_HLT_Photon200,
                     const bool &_HLT_Photon175,
                     const bool &_HLT_Ele115_CaloIdVT_GsfTrkIdT)
    : run(_run),
      lumi(_lumi),
      event(_event),
      Pileup_nTrueInt(_Pileup_nTrueInt),
      genWeight(_genWeight),
      L1PreFiringWeight_Nom(_L1PreFiringWeight_Nom),
      L1PreFiringWeight_Up(_L1PreFiringWeight_Up),
      L1PreFiringWeight_Dn(_L1PreFiringWeight_Dn),
      PV_npvsGood(_PV_npvsGood),
      Flag_goodVertices(_Flag_goodVertices),
      Flag_globalSuperTightHalo2016Filter(_Flag_globalSuperTightHalo2016Filter),
      Flag_HBHENoiseFilter(_Flag_HBHENoiseFilter),
      Flag_HBHENoiseIsoFilter(_Flag_HBHENoiseIsoFilter),
      Flag_EcalDeadCellTriggerPrimitiveFilter(_Flag_EcalDeadCellTriggerPrimitiveFilter),
      Flag_BadPFMuonFilter(_Flag_BadPFMuonFilter),
      Flag_BadPFMuonDzFilter(_Flag_BadPFMuonDzFilter),
      Flag_eeBadScFilter(_Flag_eeBadScFilter),
      Flag_ecalBadCalibFilter(_Flag_ecalBadCalibFilter),
      rho(_rho),
      HLT_IsoMu27(_HLT_IsoMu27),
      HLT_IsoMu24(_HLT_IsoMu24),
      HLT_IsoTkMu24(_HLT_IsoTkMu24),
      HLT_Mu50(_HLT_Mu50),
      HLT_TkMu50(_HLT_TkMu50),
      HLT_TkMu100(_HLT_TkMu100),
      HLT_OldMu100(_HLT_OldMu100),
      HLT_Ele27_WPTight_Gsf(_HLT_Ele27_WPTight_Gsf),
      HLT_Ele35_WPTight_Gsf(_HLT_Ele35_WPTight_Gsf),
      HLT_Ele32_WPTight_Gsf(_HLT_Ele32_WPTight_Gsf),
      HLT_Photon200(_HLT_Photon200),
      HLT_Photon175(_HLT_Photon175),
      HLT_Ele115_CaloIdVT_GsfTrkIdT(_HLT_Ele115_CaloIdVT_GsfTrkIdT)
{
}

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
    : nLHEParticles(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi)
{
}

Muons::Muons(const RVec<float> &_pt,
             const RVec<float> &_eta,
             const RVec<float> &_phi,
             const RVec<bool> &_tightId,
             const RVec<UChar_t> &_highPtId,
             const RVec<float> &_pfRelIso04_all,
             const RVec<float> &_tkRelIso)
    : size(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi),
      tightId(_tightId),
      highPtId(_highPtId),
      pfRelIso04_all(_pfRelIso04_all),
      tkRelIso(_tkRelIso)
{
}

Electrons::Electrons(const RVec<float> &_pt,
                     const RVec<float> &_eta,
                     const RVec<float> &_phi,
                     const RVec<int> &_cutBased,
                     const RVec<bool> &_cutBased_HEEP,
                     const RVec<float> &_deltaEtaSC)
    : size(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi),
      cutBased(_cutBased),
      cutBased_HEEP(_cutBased_HEEP),
      deltaEtaSC(_deltaEtaSC)
{
}

Photons::Photons(const RVec<float> &_pt,
                 const RVec<float> &_eta,
                 const RVec<float> &_phi,
                 const RVec<int> &_cutBased,
                 const RVec<bool> &_pixelSeed,
                 const RVec<bool> &_isScEtaEB,
                 const RVec<bool> &_isScEtaEE)
    : size(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi),
      cutBased(_cutBased),
      pixelSeed(_pixelSeed),
      isScEtaEB(_isScEtaEB),
      isScEtaEE(_isScEtaEE)
{
}

Taus::Taus(const RVec<float> &_pt, const RVec<float> &_eta, const RVec<float> &_phi)
    : size(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi)
{
}

Jets::Jets(const RVec<float> &_pt,
           const RVec<float> &_eta,
           const RVec<float> &_phi,
           const RVec<float> &_mass,
           const RVec<int> &_jetId,
           const RVec<float> &_btagDeepFlavB,
           const RVec<int> &_hadronFlavour,
           const RVec<int> &_genJetIdx,
           const RVec<float> &_rawFactor,
           const RVec<float> &_area)
    : size(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi),
      mass(_mass),
      jetId(_jetId),
      btagDeepFlavB(_btagDeepFlavB),
      hadronFlavour((_pt.size() > 0 and _hadronFlavour.size() == 0) ? RVec<int>(_pt.size()) : _hadronFlavour),
      genJetIdx(_genJetIdx),
      rawFactor(_rawFactor),
      area(_area)
{
}

std::size_t size;
RVec<float> pt;
RVec<float> eta;
RVec<float> phi;

MET::MET(const RVec<float> &_pt, const RVec<float> &_eta, const RVec<float> &_phi)
    : size(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi)
{
}

TrgObjs::TrgObjs(const RVec<float> &_pt,
                 const RVec<float> &_eta,
                 const RVec<float> &_phi,
                 const RVec<int> &_id,
                 const RVec<int> &_filterBits)
    : size(_pt.size()),
      pt(_pt),
      eta(_eta),
      phi(_phi),
      id(_id),
      filterBits(_filterBits)
{
}

} // namespace NanoObjects
