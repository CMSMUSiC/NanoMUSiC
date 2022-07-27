#ifndef ParticleMap_hh
#define ParticleMap_hh

#include "Pxl/Pxl/interface/pxl/core.hh"
#include "Pxl/Pxl/interface/pxl/hep.hh"

#include "ParticleVector.hh"

#include <string>
#include <functional>

class ParticleMap
{
   std::string name;
   std::string kind;
   std::map< std::string, ParticleVector > m_map;
   std::map< std::string, int > m_countMap;
   std::map< std::string, std::function< double( double ) > > m_resolutionFuncMap;
   const std::map< std::string, std::function< double( pxl::Particle* ) > > m_funcMap;

   double getApproximateResolution( const std::map< std::string, int >& countMap, double sumpt, double const fudge=1 ) const;
   double getApproximateResolutionMET( const std::map< std::string, int >& countMap, double sumpt, double const fudge=1 ) const;
   double getSumPx( const std::map< std::string, int >& countMap ) const;
   double getSumPy( const std::map< std::string, int >& countMap ) const;
   double getSumEt( const std::map< std::string, int >& countMap ) const;
   void split( std::vector< pxl::Particle* > particles, std::function< bool( pxl::Particle* ) > splittingFunc, std::string name1, std::string name2 );
   double getKinematicVariable( const std::string var, const std::map< std::string, int >& countMap ) const;
   double callResolutionFunction( const std::string& name, const double& value ) const;
   double getScaleFactorError( const std::string error_type, const std::map< std::string, int >& countMap ) const;
public:
   ParticleMap( const std::map< std::string, std::vector< pxl::Particle* > > &particleMap );

   const ParticleVector& getParticleVector( std::string& name ) const;
   std::map< std::string, int > getCountMap() const;
   int getLeptonCharge( const std::map< std::string, int >& countMap ) const;
   double getSumPt( const std::map< std::string, int >& countMap ) const;
   double getMass( const std::map< std::string, int >& countMap ) const;
   double getTransverseMass( const std::map< std::string, int >& countMap ) const;
   double getInvMass( const std::map< std::string, int >& countMap ) const;
   double getMET( const std::map< std::string, int >& countMap ) const;
   double getScaleFactor( const std::map< std::string, int >& countMap ) const;
   double getScaleFactorStatError( const std::map< std::string, int >& countMap ) const;
   double getScaleFactorSystError( const std::map< std::string, int >& countMap ) const;
   std::map< std::string, int > getFakeMap( const std::map< std::string, int >& countMap ) const;
   std::map< std::string, int > getChargeFakeMap( const std::map< std::string, int >& countMap ) const;
   std::vector< double > getBinLimits( std::string distribution, std::map< std::string, int >& countMap, double min, double max, double step_size, double const fudge=1 ) const;
   double getRealResolution( const std::map< std::string, int >& countMap ) const;
   double getApproximateResolution( const std::map< std::string, int >& countMap, double const fudge=1 ) const;
};

#endif
