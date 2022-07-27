#ifndef GENRECNAMEMAP
#define GENRECNAMEMAP

#include "Tools/MConfig.hh"
#include "Tools/Tools.hh"

class GenRecNameMap {
   public:
      struct GenRecNamePair {
         GenRecNamePair( std::string const& genName = "",
                         std::string const& recName = ""
                         ) :
            GenName( genName ),
            RecName( recName )
         {}

         std::string GenName;
         std::string RecName;
         int PdgId;
      };

      typedef std::map< std::string, GenRecNamePair > NameMap;
      typedef NameMap::const_iterator const_iterator;

      GenRecNameMap( Tools::MConfig const &cfg ) :
         m_gen_rec_map( initNameMap( cfg ) )
      {}

      GenRecNamePair const &get( std::string const &object ) const {
         NameMap::const_iterator it = m_gen_rec_map.find( object );

         if( it == m_gen_rec_map.end() ) {
            std::string const err = "No name mapping found for object '" + object + "'!";
            throw Tools::config_error( err );
         } else {
            return (*it).second;
         }
      }

      const_iterator begin() const { return m_gen_rec_map.begin(); }
      const_iterator end() const { return m_gen_rec_map.end(); }

   private:
      NameMap const initNameMap( Tools::MConfig const &cfg ) const {
         NameMap GenRecMap;
         std::string genName;
         std::string recName;
         for( auto partName : Tools::getConfigParticleMap( cfg, "use", true ) ){
            genName = cfg.GetItem< std::string >( partName.first + ".Type.Gen" );
            recName = cfg.GetItem< std::string >( partName.first + ".Type.Rec" );
            GenRecMap[ partName.first ] = GenRecNamePair( genName, recName );
         }
         return GenRecMap;
      }

      std::string const m_met_type_rec;
      NameMap const m_gen_rec_map;
};

#endif /*GENRECNAMEMAP*/
