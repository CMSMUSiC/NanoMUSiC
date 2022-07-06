#ifndef JetSubtypeCriterion_hh
#define JetSubtypeCriterion_hh

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

/*
NOTE: The following EXCLUSIVELY works for JETS!!!!!! (Because the other particles do not have the UserRecord "bJetType".)
*/
class JetSubtypeCriterion : public pxl::FilterCriterionInterface<pxl::Particle>
{

   public:
   JetSubtypeCriterion(const std::string& particleType,
                       const std::string& JetSubtype1,
                       const std::string& JetSubtype2 = "",
                       double ptMin = 0.0,
                       double etaMax = 0.0)
               : _particleType(particleType),
                 _JetSubtype1(JetSubtype1),
                 _JetSubtype2(JetSubtype2),
                 _ptMin(ptMin),
                 _etaMax(etaMax) {
   }

   virtual bool operator()(const pxl::Particle& pa) const
   {
      if ( (_particleType != "" && pa.getName() != _particleType)
           || (_ptMin > 0.0 && pa.getPt() < _ptMin)
           || (_etaMax > 0.0 && std::fabs(pa.getEta()) > _etaMax) ) return false;
      if ( (_JetSubtype1 != "" || _JetSubtype2 != "")
           && !(pa.getUserRecord( "bJetType" ) == _JetSubtype1
               || pa.getUserRecord( "bJetType" ) == _JetSubtype2) ) return false;
      return true;
   }

   private:
   std::string _particleType;
   std::string _JetSubtype1;
   std::string _JetSubtype2;
   double _ptMin;
   double _etaMax;
};


#endif
