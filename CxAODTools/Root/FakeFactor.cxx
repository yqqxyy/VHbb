#include <CxAODTools/FakeFactor.h>
#include <vector>
#include "CxAODTools/ReturnCheck.h"
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

FakeFactor :: FakeFactor (ConfigStore & config, int lep_type) :
  m_config(config),
  m_debug(false),
  m_fin("")
{
  m_config.getif<bool>("debug_fake", m_debug);

  m_config.getif<std::string>("fakerate_file", m_fin);

  if(lep_type==0)m_config.getif< std::vector<std::string> >("FakeFactorSyst_El", m_sysname);
  else if(lep_type==1)m_config.getif< std::vector<std::string> >("FakeFactorSyst_Mu", m_sysname);


  //if(m_sysname.size()==0)
  m_sysname.push_back("Nominal");

}

FakeFactor :: ~FakeFactor()
{
  delete m_infile;
}

// bool FakeFactor :: initialize ()
EL::StatusCode FakeFactor::initialize(const std::string fin)
{
 
  if(m_debug) {
    for(unsigned int s=0; s<m_sysname.size(); s++) {
      Info ("FakeFactor", "syst name = %s", m_sysname.at(s).c_str());
    }
  }
  
  

  if (m_debug) Info ("FakeFactor", "fin = %s", fin.c_str());

  m_infile = TFile::Open(fin.c_str());

  m_setting = (TH1D*) m_infile->Get("setting");
  m_iso_low = m_setting->GetBinContent(1);
  m_iso_high = m_setting->GetBinContent(2);
  if(m_setting->GetBinContent(3) > 0)
    m_use_mediumid = true;
  else
    m_use_mediumid = false;
  if(m_setting->GetBinContent(4) > 0)
    m_use_tightid = true;
  else
    m_use_tightid = false;
  if(m_setting->GetBinContent(5) > 0)
    m_use_antitightid = true;
  else
    m_use_antitightid = false;

  if (m_debug) {
    Info ("FakeFactor", "iso low  = %f", m_iso_low);
    Info ("FakeFactor", "iso high = %f", m_iso_high);
    Info ("FakeFactor", "use medium id = %d", m_use_mediumid);
    Info ("FakeFactor", "use tight id  = %d", m_use_tightid);
    Info ("FakeFactor", "use antitight id  = %d", m_use_antitightid);
  }

  m_wptbin = (TH1D*) m_infile->Get("wptbin");
  m_nwpt = m_wptbin->GetNbinsX();

  for(int w=0; w<m_nwpt; w++) {
    TH1D* tmp_ptbin = (TH1D*) m_infile->Get(Form("ptbin_%d",w));
    if(w==0)
      m_npt = tmp_ptbin->GetNbinsX();
    else if(m_npt != tmp_ptbin->GetNbinsX()) {
      Error("FakeFactor()","wrong pt binning!!");
    }
    m_ptbin.push_back( tmp_ptbin );
  }

  for(unsigned int s=0; s<m_sysname.size(); s++) {

    std::vector< std::vector< TH2D* > > tmp_fakerate;
    std::vector< std::vector< TH2D* > > tmp_d0sigblcut;

    for(int w=0; w<m_nwpt; w++) {

      std::vector< TH2D* > tmp_tmp_fakerate;
      std::vector< TH2D* > tmp_tmp_d0sigblcut;

      for(int p=0; p<m_npt; p++) {
	tmp_tmp_fakerate.push_back( (TH2D*) m_infile->Get(Form("%s/fakerate_pretag_wpt%d_pt%d",m_sysname.at(s).c_str(),w,p)) );
        tmp_tmp_d0sigblcut.push_back(  (TH2D*) m_infile->Get(Form("%s/d0sigbl_cut_wpt%d_pt%d",m_sysname.at(s).c_str(),w,p)) );

      }
      tmp_fakerate.push_back(tmp_tmp_fakerate);
      tmp_d0sigblcut.push_back(tmp_tmp_d0sigblcut);

    }

    m_fakerate.push_back(tmp_fakerate);
    m_d0sigblcut.push_back(tmp_d0sigblcut);

  }

  return true;
}


bool FakeFactor :: finalize ()
{

  m_infile->Close();

  return true;
}

bool FakeFactor :: findbin (double wpt, double pt, double eta, double dphilmet, double met)
{

 m_wpt_bin = 0;

  m_wpt_bin = m_wptbin->FindBin(wpt)-1;

  if(m_wpt_bin == -1)
    m_wpt_bin = 0;
  if(m_wpt_bin == m_nwpt)
    m_wpt_bin = m_nwpt-1;

  if(m_wpt_bin==0)
    m_v3rd = dphilmet;
  else if(m_wpt_bin==1)
    m_v3rd = met;
  else {
    Error("FakeFactor()","unknown wpt bin!! only supported 0 or 1 bin");
  }
  
   m_pt_bin = m_ptbin.at(m_wpt_bin)->FindBin(pt)-1;
   
   if(m_pt_bin == -1)
     m_pt_bin = 0;
  if(m_pt_bin == m_npt)
    m_pt_bin = m_npt-1;


  m_eta = eta;
  if(m_eta < m_fakerate.at(0).at(m_wpt_bin).at(m_pt_bin)->GetXaxis()->GetXmin())
    m_eta  = m_fakerate.at(0).at(m_wpt_bin).at(m_pt_bin)->GetXaxis()->GetXmin() + 0.00001;
  if(m_eta > m_fakerate.at(0).at(m_wpt_bin).at(m_pt_bin)->GetXaxis()->GetXmax())
    m_eta  = m_fakerate.at(0).at(m_wpt_bin).at(m_pt_bin)->GetXaxis()->GetXmax() - 0.00001;

  // m_v3rd = v3rd;
  if(m_v3rd < m_fakerate.at(0).at(m_wpt_bin).at(m_pt_bin)->GetYaxis()->GetXmin())
    m_v3rd  = m_fakerate.at(0).at(m_wpt_bin).at(m_pt_bin)->GetYaxis()->GetXmin() + 0.00001;
  if(m_v3rd > m_fakerate.at(0).at(m_wpt_bin).at(m_pt_bin)->GetYaxis()->GetXmax())
    m_v3rd  = m_fakerate.at(0).at(m_wpt_bin).at(m_pt_bin)->GetYaxis()->GetXmax() - 0.00001;

  m_etav3rd_bin = m_fakerate.at(0).at(m_wpt_bin).at(m_pt_bin)->FindBin(m_eta, m_v3rd);

  return true;
}


double FakeFactor :: get_fakefactor (std::string sys)
{

  int sys_bin = 0;

  // if(sys!="Nominal") {
  //   Error("FakeFactor()","Systematics not implemented yet!!");
  // }

  for(unsigned int s=0; s<m_sysname.size(); s++) {
    if(m_sysname.at(s) == sys)
      sys_bin = s;
  }

  double fk = m_fakerate.at(sys_bin).at(m_wpt_bin).at(m_pt_bin)->GetBinContent(m_etav3rd_bin);

  return fk;
}


double FakeFactor :: get_d0cutthreshold (std::string sys)
{

  int sys_bin = 0;

  // if(sys!="Nominal") {
  //   Error("FakeFactor()","Systematics not implemented yet!!");
  // }

  for(unsigned int s=0; s<m_sysname.size(); s++) {
    if(m_sysname.at(s) == sys)
      sys_bin = s;
  }

  double d0 = m_d0sigblcut.at(sys_bin).at(m_wpt_bin).at(m_pt_bin)->GetBinContent(m_etav3rd_bin);

  return d0;
}


bool FakeFactor :: set_variables (double etcone, double pt, double d0sigbl, bool mediumid, bool tightid )
{

  m_etcone = etcone;
  m_pt = pt;
  m_d0sigbl = d0sigbl;
  m_mediumid = mediumid;
  m_tightid = tightid;

  return true;
}


bool FakeFactor :: pass_denominator (std::string sys)
{

  // only applied to electrons passing trigger matching and VH loose electron

  bool pass_denominator = false;
  bool pass_d0cut = false;
  bool pass_iso = false;
  bool pass_id = false;

  if(m_etcone/m_pt > m_iso_low && m_etcone/m_pt < m_iso_high)
    pass_iso = true;

  if((!m_use_mediumid || m_mediumid)
     &&
     (!m_use_tightid || m_tightid)
     &&
     (!m_use_antitightid || !m_tightid))
    pass_id = true;

  double d0cut = get_d0cutthreshold(sys);
  if(fabs(m_d0sigbl) < d0cut)
    pass_d0cut = true;

  if(pass_d0cut &&
     pass_iso &&
     pass_id)
    pass_denominator = true;

  return pass_denominator;

}

bool FakeFactor :: set_parameters (double wpt, double pt, double eta, double dphilmet, double met, double etcone, double d0sigbl, bool mediumid, bool tightid)
{
  
  findbin (wpt, pt, eta, dphilmet, met);
  set_variables (etcone, pt, d0sigbl, mediumid, tightid);

  return true;
}


double FakeFactor :: get_weight (std::string sys)
{

  double weight = 0;
  if(pass_denominator(sys))
    weight = get_fakefactor(sys);

  return weight;
}

bool FakeFactor :: qcd_selection ()
{

  bool doqcd;
  m_config.getif<bool>("doElQCD", doqcd);

  return doqcd;

}
