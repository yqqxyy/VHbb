#ifndef CxAODTools_Utilities_H
#define CxAODTools_Utilities_H

#include <utility>
#include <type_traits>
#include <tuple>
#include <map>
#include <string>
#include <cstring>

#include <TString.h>

// EDM include(s)
#include "AthContainers/AuxElement.h"

namespace Analysis {
// utility for getting a list of all associated AuxElements (name, type)
std::map<std::string, std::string> getAssociatedAuxElements (const SG::AuxElement&);
std::map<std::string, std::string> getAssociatedAuxElements (const SG::AuxElement*);

// replaces all occurrences of "from" to "to"
std::string replace_all(std::string str, const std::string& from, const std::string& to);

// returns true if string is a "number"
bool is_numeric (const std::string& input);

// tokenize string
std::vector<std::string> tokenize (const std::string &input, const char &delimiter);

class fast_string_cmp {
public:
  bool operator() (const char *a, const char *b) const {
    unsigned n = strlen(a);
    unsigned m = strlen(b);

    if (n < m)
      return true;
    if (m < n)
      return false;
    return memcmp(a, b, n) < 0 ? true : false;
  }
  bool operator() (const std::string &a, const std::string &b) const {
    unsigned n = a.size();
    unsigned m = b.size();

    if (n < m)
      return true;
    if (m < n)
      return false;
    return memcmp(a.c_str(), b.c_str(), n) < 0 ? true : false;
  }
  bool operator() (const TString &a, const TString &b) const {
    unsigned n = a.Length();
    unsigned m = b.Length();

    if (n < m)
      return true;
    if (m < n)
      return false;
    return memcmp(a.Data(), b.Data(), n) < 0 ? true : false;
  }
};



// tuple for_each (iterate over an std::tuple))
template<typename FuncT, std::size_t I = 0, typename ... Tp>
inline typename std::enable_if<I == sizeof ... (Tp), void>::type
for_each (std::tuple<Tp ...>&, FuncT&&)
{}

template<typename FuncT, std::size_t I = 0, typename ... Tp>
inline typename std::enable_if<I == sizeof ... (Tp), void>::type
for_each (std::tuple<Tp ...>&, const FuncT&)
{}

template<typename FuncT, std::size_t I = 0, typename ... Tp>
inline typename std::enable_if < I<sizeof ... (Tp), void>::type
for_each (std::tuple<Tp ...> &t, FuncT &&f)
{
  f(std::get<I>(t));
  Analysis::for_each<FuncT, I + 1, Tp ...>(t, std::forward<FuncT>(f));
}

template<typename FuncT, std::size_t I = 0, typename ... Tp>
inline typename std::enable_if < I<sizeof ... (Tp), void>::type
for_each (std::tuple<Tp ...> &t, const FuncT &f)
{
  f(std::get<I>(t));
  Analysis::for_each<FuncT, I + 1, Tp ...>(t, f);
}


// retrieve the index (for std::get) of a tuple by type
//  usage: std::get<Analysis::type_index<0, Type, Types ...>::type::index>(tuple)
//  TODO: should make this tidier to use
template<int Index, class Search, class First, class ... Types>
struct type_index
{
  typedef typename Analysis::type_index<Index + 1, Search, Types ...>::type type;
  static constexpr int index = Index;
};

template<int Index, class Search, class ... Types>
struct type_index<Index, Search, Search, Types ...>
{
  typedef type_index type;
  static constexpr int index = Index;
};

}

#endif // ifndef CxAODTools_Utilities_H
