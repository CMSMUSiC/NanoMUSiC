#include <istream>
#include "dCacheBuf.hh"

class idCacheStream : public std::istream {
public:
   idCacheStream() : std::istream(), buf() {
      //tell the base class istream where to find the buffer
      rdbuf( &buf );
   }
   //construct the stream and at the same time open a file
   //make is explicit, there is no sane way to cast (!) a char[] into a idCacheStream
   explicit idCacheStream( const char *filename ) : std::istream(), buf() {
      //tell the base class istream where to find the buffer
      rdbuf( &buf );
      buf.open( filename );
   }

   //tell us, if there is a file already open
   bool is_open(){ return buf.is_open(); }
   //open the file denoted by name
   //name must something that's understood by dCache (precisely by dc_open() )
   void open( const char *filename, unsigned int timeout=3600 ){
      //try to open the file
      if( buf.open( filename, timeout ) ){
         //ok, clear residual error states
         clear();
      } else {
         //not ok, set bad bit
         setstate( badbit );
      }
   }
   //close file, if there is one
   void close(){
      buf.close();
      //clear residual error states
      clear();
   }

private:
   //this thing manages the file and the buffer and reads data
   dCacheBuf buf;
};
