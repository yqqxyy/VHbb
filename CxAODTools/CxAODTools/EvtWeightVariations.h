#ifndef __EvtWeightVariations_h__
#define __EvtWeightVariations_h__

#include "xAODEventInfo/EventInfo.h"
#include <map>
#include <unordered_set>
#include <unordered_map>

class EvtWeightVariations{
 public:
  EvtWeightVariations( const int mode ) {

    m_listOfVariations_SignalMiNLO.clear();
    m_listOfVariations_SignalGG.clear();
    m_listOfVariations_SherpaZmumu221.clear();
    m_listOfVariations_SherpaV.clear();
    m_listOfVariations_PowhegPythia8Ttbar.clear();
    m_listOfVariations_PowhegPythia8Stop.clear();

    initSignalMiNLO( mode );
    initSignalGG( mode );
    initSherpaZmumu221();
    initSherpaV( mode ); 
    initPowhegPythia8Ttbar( mode ); 
    initPowhegPythia8Stop( mode ); 

  };
  ~EvtWeightVariations(){};
  
 public:
  bool  hasVariation(const std::string &SYSNAME, const int DSID );
  float getWeightVariation( const xAOD::EventInfo* eventInfo, const std::string &SYSNAME );
  float GetTtbarRadHiNewNominal(const xAOD::EventInfo* eventInfo);
  std::unordered_set<std::string> getListOfVariations( const int DSID );

 private:
  void initSignalMiNLO( const int mode );
  void initSignalGG( const int mode );
  void initSherpaZmumu221();
  void initSherpaV( const int mode );
  void initPowhegPythia8Ttbar( const int mode );
  void initPowhegPythia8Stop( const int mode );

  std::unordered_map<std::string, int> m_variations_SignalMiNLO;
  std::unordered_map<std::string, int> m_variations_SignalGG;
  std::unordered_map<std::string, int> m_variations_SherpaZmumu221;
  std::unordered_map<std::string, int> m_variations_SherpaV; 
  std::unordered_map<std::string, int> m_variations_PowhegPythia8Ttbar;
  std::unordered_map<std::string, int> m_variations_PowhegPythia8Stop;

  std::unordered_set<std::string> m_listOfVariations_SignalMiNLO;
  std::unordered_set<std::string> m_listOfVariations_SignalGG;
  std::unordered_set<std::string> m_listOfVariations_SherpaZmumu221;
  std::unordered_set<std::string> m_listOfVariations_SherpaV;
  std::unordered_set<std::string> m_listOfVariations_PowhegPythia8Ttbar;
  std::unordered_set<std::string> m_listOfVariations_PowhegPythia8Stop;

 public:
  bool isSignalMiNLO( int DSID );
  bool isSignalGG( int DSID );
  bool isSherpaZmumu221( int DSID );
  bool isSherpaV( int DSID );
  bool isPowhegPythia8Ttbar( int DSID );
  bool isPowhegPythia8Stop( int DSID );
};

#endif
