#ifndef SKIPEVENTS
#define SKIPEVENTS

#include <map>
#include <set>
#include <string>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/filesystem.hpp>
#pragma GCC diagnostic pop

namespace Tools {
   class MConfig;
}

class SkipEvents {
   public:
      typedef boost::filesystem::path Path;
      typedef std::vector< Path > Paths;
      typedef std::set< unsigned int > Events;
      typedef std::map< unsigned int, Events > LumiSections;
      typedef std::map< int, LumiSections > RunsLumisEvents;

      explicit SkipEvents( Tools::MConfig const &cfg );
      ~SkipEvents() {}

      bool skip( unsigned int const runNumber,
                 unsigned int const lumiSection,
                 unsigned int const eventNumber
                 );

   private:
      Paths initFilePaths( Tools::MConfig const &cfg ) const;
      RunsLumisEvents initRunsLumisEvents() const;

      Paths const m_fileList;

      RunsLumisEvents const m_skipRunsLumisEvents;

      // Cache the last runNumber and lumiSection because they were not in the
      // skip events file. When the next event with this runNumber and
      // lumiSection is checked, it is instantly passed because it is known,
      // that runNumber and/or lumiSection are not in the file and thus not
      // events from this runNumber/lumiSection must be skipped.
      // In this way the map (with the events to be skipped) is not scanned
      // every time for the same runNumber/lumiSection that is not there.
      // It assumes that the runNumbers and lumiSections (i.e. events) come in
      // an ordered way.
      std::pair< unsigned int, unsigned int > m_dontSkip;
};

#endif /*SKIPEVENTS*/
