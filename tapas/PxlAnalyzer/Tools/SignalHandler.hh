#ifndef Tools_SignalHandler_hh
#define Tools_SignalHandler_hh

#include <map>
#include <set>
#include <csignal>

namespace Tools {
   //interface to inherit from if you want to handle signals
   class EventHandler {
   public:
      //overload this
      virtual void handle_signal( int signum ) = 0;
   };

   //Exactly one object of this class exists and handles all signals
   class SignalHandler {
   public:
      //get the singleton object
      static SignalHandler* handler();

      //Register an EventHandler object to be called for a signal
      void register_handler( int signum, EventHandler *eh );

      //Remove the handler for a certain signal
      void remove_handler( int signum, EventHandler *eh );

   private:
      //This a singleton, so no public constructor
      SignalHandler() {}
      SignalHandler( const SignalHandler & in) {}
      ~SignalHandler() {}

      //Where are we
      static SignalHandler *m_instance;

      //This function is called for any signal
      static void dispatcher( int signum );

      //stores a set of handles for any signal
      static std::map< int, std::set< EventHandler* > > m_signal_handlers;
      //stores the state before a signal was bound to a handler
      //used to restore the state later
      static std::map< int, sighandler_t > m_signal_old_handler;
   };
}

#endif
