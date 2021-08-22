
#ifndef CxAODTools__VBFHbb_JetAssignment_H
#define CxAODTools__VBFHbb_JetAssignment_H

#include "vector"
#include "xAODJet/Jet.h"
#include "EventLoop/Algorithm.h"

class jetAssignmentTool {
 public:
  jetAssignmentTool();
  jetAssignmentTool(jetAssignmentTool const&);
  ~jetAssignmentTool();

  EL::StatusCode setAssignmentType(std::string);
  EL::StatusCode setIsMC(bool);
  //  EL::StatusCode setDataPeriod(std::string);
  EL::StatusCode setSelection(std::string selection);
  void setDebugMode(bool debugMode = true);

  bool assignJets(std::vector<xAOD::Jet const*> const&);

  const std::string assignmentType() const;
  std::vector<xAOD::Jet const*> signalJets() const;
  std::vector<xAOD::Jet const*> vbfJets() const;

 protected:
  void clear();
  void printResults();
  bool assign0tag_4cen(std::vector<xAOD::Jet const*>&);
  bool assignBtag_4cen(std::vector<xAOD::Jet const*> &inputJets, std::string WP);
  bool assign0tag_2cen(std::vector<xAOD::Jet const*>&);
  bool assignBtag_2cen(std::vector<xAOD::Jet const*> &inputJets, std::string WP);
  bool assignBtag_2cen_reverse(std::vector<xAOD::Jet const*> &inputJets, std::string WP);

 private:
  std::string _type;
  //  std::string _Period;
  std::string _Selection;

  std::vector<xAOD::Jet const*> _signalJets;
  std::vector<xAOD::Jet const*> _vbfJets;

  bool _debugMode;
  bool m_isMC; 

 private:
  static constexpr double m_b_85_weight = 0.110;
  static constexpr double m_b_77_weight = 0.645;
  static constexpr double m_b_70_weight = 0.831;
  static constexpr double m_b_60_weight = 0.939;
};

#include "VBFHbbJetAssignment.icc"

#endif 

/* b-tagging bench mark numbers */
/* https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/BTaggingBenchmarksRelease21 */
/* recommendation is MV2c10 for r21 */
/* eff 60 wp 0.939 */
/* eff 70 wp 0.831 */
/* eff 77 wp 0.645 */
/* eff 85 wp 0.110 */
