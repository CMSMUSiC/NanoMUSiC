#ifndef RunLumiRanges_hh
#define RunLumiRanges_hh

#include <utility>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <exception>


namespace lumi {

   class bad_config : public std::exception {
   public:
      bad_config() : message() {}
      bad_config( const std::string &msg ) : message( msg ) {}
      bad_config( const std::string &filename, const std::string &msg ) {
         message = "Error in file "+filename+": "+msg;
      }
      bad_config( const std::string &filename, const char c ) {
         message = "Unexpected character '";
         message += c;
         message += "' in file ";
         message += filename;
      }
      ~bad_config() throw() {}

      virtual const char* what() const throw() {
         return ("Syntax error in the lumi ranges configuration: "+message).c_str();
      }

   private:
      std::string message;
   };




   typedef unsigned long ID;

   class LumiRanges {
   public:
      typedef std::pair< ID, ID > range;

      LumiRanges() : ranges() {}

      void addRange( ID min, ID max );

      bool check( const ID LS ) const;


   private:
      std::vector< range > ranges;
   };


   class RunLumiRanges {
   public:
      explicit RunLumiRanges( const std::string &filename );

      bool check( const ID run, const ID LS );

   private:
      bool empty, first;

      std::map< ID, LumiRanges > ranges;

      ID last_run, last_LS;
      bool last_state;
      std::map< ID, LumiRanges >::const_iterator last_range;
   };
}

#endif
