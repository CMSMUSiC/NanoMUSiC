#include "MConfig.hh"

using namespace Tools;
using namespace std;

bool MConfig::CheckItem(const string &key)
{
   map<string, string>::const_iterator it = m_configMap.find(key);

   if (it == m_configMap.end())
   {
      return false;
   }

   return true;
}

void MConfig::Print()
{
   int width = FindLongestKey();

   cout << "-------- Content of Config Map from file '" << m_configFileName << "' ---------" << endl;
   for (map<string, string>::iterator it = m_configMap.begin(); it != m_configMap.end(); ++it)
   {
      cout << setw(width) << setiosflags(ios::left)
           << (*it).first << " = " << (*it).second << endl;
   }
   cout << endl;
}

void MConfig::DumpToFile(std::string filePath)
{

   ofstream dumpFile;
   dumpFile.open(filePath);

   int width = FindLongestKey();

   for (map<string, string>::iterator it = m_configMap.begin(); it != m_configMap.end(); ++it)
   {
      dumpFile << setw(width) << setiosflags(ios::left)
               << (*it).first << " = " << (*it).second << endl;
   }
   dumpFile << endl;
   dumpFile.close();
}

void MConfig::setYear(std::string year, std::vector<std::string> possibleYears)
{

   // define and remove undesirable years
   auto undesirableYears = possibleYears;
   undesirableYears.erase(std::remove(undesirableYears.begin(), undesirableYears.end(), year), undesirableYears.end());

   // add "." to each year
   year = year + ".";
   for (auto &uy : undesirableYears)
   {
      uy = uy + ".";
   }

   for (auto const &u_year : undesirableYears)
   {
      for (auto const &item_ : m_configMap)
      {
         auto key = item_.first;
         if (key.find(u_year) == 0)
         {
            RemoveItem(key);
         }
      }
   }

   // check and remove duplicates
   // Example:
   // <year>.foo.bar = 123
   // foo.bar = 123 <-- this should be removed
   for (auto const &item_ : m_configMap)
   {
      auto key = item_.first;
      if (key.find(year, 0) == 0)
      {
         auto temp_key = key;
         boost::replace_first(temp_key, year, "");
         if (CheckItem(temp_key))
         {
            RemoveItem(temp_key);
         }
      }
   }

   // remove the desired year
   // Example:
   // <year>.foo.bar = 123 --> foo.bar = 123
   std::vector<std::pair<std::string, std::string>> to_add;
   std::vector<std::string> to_remove;
   for (auto const &[key, val] : m_configMap)
   {
      // std::cout << key << " : " << val << std::endl;
      if (key.find(year, 0) == 0)
      {
         auto temp_key = key;
         boost::replace_first(temp_key, year, "");
         to_add.push_back({temp_key, val});
         to_remove.push_back(key);
      }
   }

   // remove stuff
   for (auto const &kv : to_remove)
   {
      RemoveItem(kv);
   }

   // add stuff
   for (auto const &kv : to_add)
   {
      AddItem(kv.first, kv.second);
   }
}

bool MConfig::RemoveItem(const string &itemtag)
{
   if (!m_configMap.erase(itemtag))
   {
      cerr << "WARNING: MConfig::RemoveItem( const string &itemtag ): Could not remove, item '"
           << itemtag << "' not found!" << endl;
      return false;
   }
   else
      return true;
}
