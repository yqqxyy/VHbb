#ifndef CxAODTools_FakeFactor_H
#define CxAODTools_FakeFactor_H

#include "CxAODTools/ConfigStore.h"
#include "EventLoop/StatusCode.h"
class TFile;
class TH1D;
class TH2D;

class FakeFactor
{

 protected:
  ConfigStore m_config; //!
  bool m_debug;
  std::string m_fin;//!
  std::vector< std::string > m_sysname;


  int m_nwpt;//!
  int m_npt;//!

  TFile* m_infile;
  TH1D* m_setting;
  std::vector< std::vector< std::vector< TH2D* > > > m_fakerate;
  std::vector< std::vector< std::vector< TH2D* > > > m_d0sigblcut;
  TH1D* m_wptbin;//!
  std::vector< TH1D* > m_ptbin;

  int m_wpt_bin;//!
  int m_pt_bin;//!
  int m_etav3rd_bin;//!

  double m_eta;//!
  double m_v3rd;//!

  double m_iso_low;
  double m_iso_high;
  double m_d0cut;
  bool m_use_mediumid;
  bool m_use_tightid;
  bool m_use_antitightid;

  double m_etcone;
  double m_pt;
  double m_d0sigbl;
  bool m_mediumid;
  bool m_tightid;



 public:

  FakeFactor (ConfigStore & config, int lep_type);
  virtual ~FakeFactor();
  /* bool initialize(); */
  virtual EL::StatusCode initialize(const std::string fin);
  /* //FakeFactor() = delete; */

  bool finalize();
  bool findbin (double wpt, double pt, double eta, double dphilmet, double met);
  bool set_variables (double etcone, double pt, double d0sigbl, bool mediumid, bool tightid);
  double get_fakefactor(std::string sys);
  double get_d0cutthreshold(std::string sys);
  bool pass_denominator (std::string sys);
  bool qcd_selection();

  bool set_parameters (double wpt, double pt, double eta, double dphilmet, double met, double etcone, double d0sigbl, bool mediumid, bool tightid);
  double get_weight (std::string sys);

};

#endif
