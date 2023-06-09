#ifndef EVENTVIEW_HPP
#define EVENTVIEW_HPP

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "fmt/format.h"

#include "Math/Vector4D.h"
#include "Math/Vector4Dfwd.h"
#include "Math/VectorUtil.h"
#include "ROOT/RVec.hxx"

using namespace ROOT;
using namespace ROOT::Math;
using namespace ROOT::VecOps;

#define DECLARE_VARIABLE(type, name) std::shared_ptr<type> m_##name;

#define DECLARE_GETTER(type, name)                                                                                     \
    auto get_##name()->type const                                                                                      \
    {                                                                                                                  \
        return *m_##name;                                                                                              \
    }

#define DECLARE_SETTER(type, name)                                                                                     \
    auto set_##name(const type &value)->void                                                                           \
    {                                                                                                                  \
        m_##name = std::make_shared<type>(value);                                                                      \
    }

#define DECLARE_SHARED_SETTER(type, name)                                                                              \
    auto set_shared_##name(const type &value)->void                                                                    \
    {                                                                                                                  \
        *m_##name = value;                                                                                             \
    }

#define ADD_VAR(type, name)                                                                                            \
  private:                                                                                                             \
    DECLARE_VARIABLE(type, name)                                                                                       \
  public:                                                                                                              \
    DECLARE_GETTER(type, name)                                                                                         \
    DECLARE_SHARED_SETTER(type, name)                                                                                  \
    DECLARE_SETTER(type, name)

class EventView
{
    //    private:
    //     std::shared_ptr<double> m_weight;
    //     std::shared_ptr<std::vector<float>> m_pts;

    //    public:
    //     auto set_shared_weight(double weight) -> void { *m_weight = weight; }
    //     auto set_weight(double weight) -> void {
    //         m_weight = std::make_shared<double>(weight);
    //     }
    //     auto get_weight() -> double const { return *m_weight; }
    //     auto get_pts() -> std::vector<float> const { return *m_pts; }

    using ScaleFactor_t = std::unordered_map<std::string, RVec<double>>;
    using pair_of_doubles = std::pair<double, double>;

    ADD_VAR(unsigned int, Run)
    ADD_VAR(unsigned int, LumiSection)
    ADD_VAR(unsigned long, EventNum)
    ADD_VAR(std::string, Dataset)
    ADD_VAR(std::string, Filename)
    ADD_VAR(unsigned int, EventNumPxlio)
    ADD_VAR(double, genWeight)
    ADD_VAR(double, PUWeight)
    ADD_VAR(double, PUWeightUp)
    ADD_VAR(double, PUWeightDown)
    ADD_VAR(std::string, Process)
    ADD_VAR(std::string, scale_variation)
    ADD_VAR(int, scale_variation_n)
    ADD_VAR(std::vector<float>, pdf_weights)
    ADD_VAR(pair_of_doubles, as_weights)
    ADD_VAR(double, Global_ScalefactorError)
    ADD_VAR(double, prefiring_scale_factor)
    ADD_VAR(double, prefiring_scale_factor_up)
    ADD_VAR(double, prefiring_scale_factor_down)
    ADD_VAR(bool, filter_accept)
    ADD_VAR(bool, Veto)
    ADD_VAR(bool, trigger_accept)
    ADD_VAR(bool, generator_accept)
    ADD_VAR(bool, topo_accept)
    ADD_VAR(RVec<Math::PtEtaPhiMVector>, electrons)
    ADD_VAR(ScaleFactor_t, electrons_scalefactors)
    ADD_VAR(RVec<bool>, electrons_matches)
    ADD_VAR(RVec<Math::PtEtaPhiMVector>, muons)
    ADD_VAR(ScaleFactor_t, muons_scalefactors)
    ADD_VAR(RVec<bool>, muons_matches)
    ADD_VAR(RVec<Math::PtEtaPhiMVector>, photons)
    ADD_VAR(ScaleFactor_t, photons_scalefactors)
    ADD_VAR(RVec<bool>, photons_matches)
    ADD_VAR(RVec<Math::PtEtaPhiMVector>, jets)
    ADD_VAR(ScaleFactor_t, jets_scalefactors)
    ADD_VAR(RVec<bool>, jets_matches)
    ADD_VAR(RVec<Math::PtEtaPhiMVector>, bjets)
    ADD_VAR(ScaleFactor_t, bjets_scalefactors)
    ADD_VAR(RVec<bool>, bjets_matches)
    ADD_VAR(RVec<Math::PtEtaPhiMVector>, met)
    ADD_VAR(ScaleFactor_t, met_scalefactors)
    ADD_VAR(RVec<bool>, met_matches)

  public:
    EventView(unsigned int Run,
              unsigned int LumiSection,
              unsigned long EventNum,
              const std::string &Dataset,
              const std::string &Filename,
              unsigned int EventNumPxlio,
              double genWeight,
              double PUWeight,
              double PUWeightUp,
              double PUWeightDown,
              const std::string &Process,
              const std::string &scale_variation,
              int scale_variation_n,
              const std::vector<float> &pdf_weights,
              const pair_of_doubles &as_weights,
              double Global_ScalefactorError,
              double &prefiring_scale_factor,
              double &prefiring_scale_factor_up,
              double &prefiring_scale_factor_down,
              bool filter_accept,
              bool Veto,
              bool trigger_accept,
              bool generator_accept,
              bool topo_accept,
              const RVec<Math::PtEtaPhiMVector> &electrons,
              const ScaleFactor_t &electrons_scalefactors,
              const RVec<bool> &electrons_matches,
              const RVec<Math::PtEtaPhiMVector> &muons,
              const ScaleFactor_t &muons_scalefactors,
              const RVec<bool> &muons_matches,
              const RVec<Math::PtEtaPhiMVector> &photons,
              const ScaleFactor_t &photons_scalefactors,
              const RVec<bool> &photons_matches,
              const RVec<Math::PtEtaPhiMVector> &jets,
              const ScaleFactor_t &jets_scalefactors,
              const RVec<bool> &jets_matches,
              const RVec<Math::PtEtaPhiMVector> &bjets,
              const ScaleFactor_t &bjets_scalefactors,
              const RVec<bool> &bjets_matches,
              const RVec<Math::PtEtaPhiMVector> &met,
              const ScaleFactor_t &met_scalefactors,
              const RVec<bool> &met_matches)
        : m_Run(std::make_shared<unsigned int>(Run)),
          m_LumiSection(std::make_shared<unsigned int>(LumiSection)),
          m_EventNum(std::make_shared<unsigned long>(EventNum)),
          m_Dataset(std::make_shared<std::string>(Dataset)),
          m_Filename(std::make_shared<std::string>(Filename)),
          m_EventNumPxlio(std::make_shared<unsigned int>(EventNumPxlio)),
          m_genWeight(std::make_shared<double>(genWeight)),
          m_PUWeight(std::make_shared<double>(PUWeight)),
          m_PUWeightUp(std::make_shared<double>(PUWeightUp)),
          m_PUWeightDown(std::make_shared<double>(PUWeightDown)),
          m_Process(std::make_shared<std::string>(Process)),
          m_scale_variation(std::make_shared<std::string>(scale_variation)),
          m_scale_variation_n(std::make_shared<int>(scale_variation_n)),
          m_pdf_weights(std::make_shared<std::vector<float>>(pdf_weights)),
          m_as_weights(std::make_shared<pair_of_doubles>(as_weights)),
          m_Global_ScalefactorError(std::make_shared<double>(Global_ScalefactorError)),
          m_prefiring_scale_factor(std::make_shared<double>(prefiring_scale_factor)),
          m_prefiring_scale_factor_up(std::make_shared<double>(prefiring_scale_factor_up)),
          m_prefiring_scale_factor_down(std::make_shared<double>(prefiring_scale_factor_down)),
          m_filter_accept(std::make_shared<bool>(filter_accept)),
          m_Veto(std::make_shared<bool>(Veto)),
          m_trigger_accept(std::make_shared<bool>(trigger_accept)),
          m_generator_accept(std::make_shared<bool>(generator_accept)),
          m_topo_accept(std::make_shared<bool>(topo_accept)),
          m_electrons(std::make_shared<RVec<Math::PtEtaPhiMVector>>(electrons)),
          m_electrons_scalefactors(std::make_shared<ScaleFactor_t>(electrons_scalefactors)),
          m_electrons_matches(std::make_shared<RVec<bool>>(electrons_matches)),
          m_muons(std::make_shared<RVec<Math::PtEtaPhiMVector>>(muons)),
          m_muons_scalefactors(std::make_shared<ScaleFactor_t>(muons_scalefactors)),
          m_muons_matches(std::make_shared<RVec<bool>>(muons_matches)),
          m_photons(std::make_shared<RVec<Math::PtEtaPhiMVector>>(photons)),
          m_photons_scalefactors(std::make_shared<ScaleFactor_t>(photons_scalefactors)),
          m_photons_matches(std::make_shared<RVec<bool>>(photons_matches)),
          m_jets(std::make_shared<RVec<Math::PtEtaPhiMVector>>(jets)),
          m_jets_scalefactors(std::make_shared<ScaleFactor_t>(jets_scalefactors)),
          m_jets_matches(std::make_shared<RVec<bool>>(jets_matches)),
          m_bjets(std::make_shared<RVec<Math::PtEtaPhiMVector>>(bjets)),
          m_bjets_scalefactors(std::make_shared<ScaleFactor_t>(bjets_scalefactors)),
          m_bjets_matches(std::make_shared<RVec<bool>>(bjets_matches)),
          m_met(std::make_shared<RVec<Math::PtEtaPhiMVector>>(met)),
          m_met_scalefactors(std::make_shared<ScaleFactor_t>(met_scalefactors)),
          m_met_matches(std::make_shared<RVec<bool>>(met_matches))
    {
    }

    EventView clone()
    {
        auto new_event_view = *this;
        return new_event_view;
    }
};

#endif