
#include <algorithm>
#include <sstream>

#include "CxAODTools/Utilities.h"

using std::map;
using std::string;
using std::all_of;
using SG::AuxElement;

namespace Analysis {
map<string, string> getAssociatedAuxElements (const AuxElement &e) {
  return getAssociatedAuxElements(&e);
}

map<string, string> getAssociatedAuxElements (const AuxElement *e) {
  map<string, string> result;
  static auto &reg = SG::AuxTypeRegistry::instance();

  for (auto auxid : e->getAuxIDs()) result[reg.getName(auxid)] = reg.getTypeName(auxid);

  // reg.getName(auxid);
  // reg.getType(auxid);
  // reg.getTypeName(auxid);
  return result;
} // getAuxAssociatedElements


string replace_all(string str, const string& from, const string& to) {
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
  return str;
}

bool is_numeric(const string& input) {
      return all_of(input.begin(), input.end(), ::isdigit);
}

std::vector<std::string> tokenize (const std::string &input, const char &delimiter) 
{
  std::vector<std::string> v;
  std::istringstream buf(input);
  for(std::string token; std::getline(buf, token, delimiter); )
    v.push_back(token);
  return v;
}

}
