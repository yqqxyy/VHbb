#include "CxAODReader_VHbb/VHResRegions.h"

#include <TDirectory.h>
#include <TFile.h>
#include <TH1.h>

#include <iostream>

#include "EventLoop/IWorker.h"

std::map<VHResRegions::Label, std::string> VHResRegions::m_labelNames{
    {VHResRegions::Label::NONE, ""},
    {VHResRegions::Label::SR, "SR_"},
    {VHResRegions::Label::mBBCR, "mbbCR_"},
    {VHResRegions::Label::mBBCRlow, "mbbCRlow_"},
    {VHResRegions::Label::mBBCRhigh, "mbbCRhigh_"},
    {VHResRegions::Label::mbbPresel, "mbbPresel_"},
    {VHResRegions::Label::mbbIncl, "mbbIncl_"},
};

std::string VHResRegions::regName(int ntag, int naddTag, float pTV, float mVH,
                                  Label label) {
  std::string name;
  if (ntag < 0)
    name += "0p";
  else if (ntag == 0)
    name += "0";
  else if (ntag == 1)
    name += "1";
  else if (ntag == 2)
    name += "2";
  else
    name += "3p";
  name += "tag";

  name += "0pjet";

  // FOR BOOSTED ANALYSIS
  // MET
  if (pTV > 150e3) {
    name += "_150ptv_";
  }

  // mVH
  if (mVH < 0)
    name += "";
  else if (mVH < 500e3)
    name += "0_500mVH_";
  else
    name += "500mVH_";

  // ADD BTAG
  if (naddTag >= 0) {
    if (naddTag == 0)
      name += "0btrkjetunmatched_";
    else
      name += "1pbtrkjetunmatched_";
  }

  name += m_labelNames[label];

  return name;
}

int VHResRegions::regCode(int ntag, int naddTag, float pTV, float mVH,
                          Label label) {
  // quite dumb, but it should work well
  if (ntag > 3) {
    ntag = 3;
  }
  int tagcode = (ntag + 4) % 10;
  if (naddTag > 1) {
    naddTag = 1;
  }
  int njetcode = (naddTag + 4) % 10;
  int pTVcode = 5;
  if (pTV <= 150e3) pTVcode = 0;
  int mVHcode = 0;
  if (mVH < 500e3) {
    mVHcode = 1;
  }
  if (mVH < 0) {
    mVHcode = 2;
  }
  int labelcode = (int)(label);

  int code = tagcode + 10 * njetcode + 100 * pTVcode + 1000 * mVHcode +
             10000 * labelcode;
  return code;
}

void VHResRegions::writeHistToFile(TH1* h, const std::string& syst,
                                   const std::string& sample,
                                   const std::string& regName, TFile* f) {
  if (!f) { /* shout something*/
    return;
  }
  if (!h) { /* shout something*/
    return;
  }

  // first, set the name
  // std::cout << "working on " << syst << " " << sample << " " << regName << "
  // " << h->GetName()  << std::endl;
  std::string hname = sample + "_" + regName + h->GetName();
  // std::cout << "working on " << hname << std::endl;
  if (syst != "Nominal") {
    hname += ("_" + syst);
  }
  h->SetName(hname.c_str());

  // then move to the right place
  if (syst != "Nominal") {
    TDirectory* subdir = f->GetDirectory(syst.c_str());
    if (!subdir) {
      subdir = f->mkdir(syst.c_str());
    }
    subdir->cd();
  }

  // Write !
  h->Write();

  // go back to where we were
  if (syst != "") {
    f->cd();
  }
  // std::cout << "end of " << hname << std::endl;
}

void VHResRegions::registerToELWorker(TH1* h, const std::string& syst,
                                      const std::string& sample,
                                      const std::string& regName,
                                      EL::IWorker* wk) {
  if (!wk) { /* shout something*/
    return;
  }
  if (!h) { /* shout something*/
    return;
  }

  // set the name
  std::string hname = sample + "_" + regName + h->GetName();
  if (syst != "Nominal") {
    hname = syst + "___" + hname;
    hname += ("_" + syst);
  }
  h->SetName(hname.c_str());
  wk->addOutput(h);
  // EL takes care of deleting histograms
}
