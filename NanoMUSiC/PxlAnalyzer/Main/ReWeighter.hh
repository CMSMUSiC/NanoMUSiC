#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

namespace Tools {
   class MConfig;
}

// CMSSW include:
#include "PhysicsTools/Utilities/interface/LumiReWeighting.h"


class ReWeighter {
public:
   ReWeighter( const Tools::MConfig &cutconfig, int i );
   ~ReWeighter() {}

   void ReWeightEvent( pxl::Event* event );
   void init();
   static void adaptConfig( Tools::MConfig& config, const std::string& filename );

private:
   std::string dataname;
   std::string mcname;
   std::string datahistname;
   std::string mchistname;
   int m_syst;
   edm::LumiReWeighting m_LumiWeights;
   const bool m_useGenWeights;
   const bool m_useREcoVertices;
   const bool m_usePileUpReWeighting;
   const bool m_quietMode;
   bool m_init;
   int m_max_gen_vertices;
};
