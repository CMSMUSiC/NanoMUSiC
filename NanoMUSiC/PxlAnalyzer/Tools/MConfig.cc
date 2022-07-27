#include "MConfig.hh"

using namespace Tools;
using namespace std;

bool MConfig::CheckItem( const string &key ) {
   map< string, string >::const_iterator it = m_configMap.find( key );

   if( it == m_configMap.end() ) return false;

   return true;
}

void MConfig::Print() {
   int width = FindLongestKey();

   cout << "-------- Content of Config Map from file '" << m_configFileName << "' ---------" << endl;
   for( map< string, string>::iterator it = m_configMap.begin(); it != m_configMap.end(); ++it ) {
      cout << setw( width ) << setiosflags( ios::left )
           << (*it).first << " = " << (*it).second << endl;
   }
   cout << endl;
}

bool MConfig::RemoveItem( const string &itemtag ) {
   if( !m_configMap.erase( itemtag ) ) {
      cerr << "WARNING: MConfig::RemoveItem( const string &itemtag ): Could not remove, item '"
           << itemtag << "' not found!" << endl;
      return false;
   } else return true;
}
