#ifndef BootstrapGenerator_h
#include "CxAODReader/BootstrapGenerator.h"
#endif

#ifndef TH2Bootstrap_h
#include "CxAODReader/TH2Bootstrap.h"
#endif

#ifndef ROOT_TH2D
#include "TH2D.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

ClassImp(TH2Bootstrap)

    //______________________________________________________________________________
    TH2Bootstrap::TH2Bootstrap()
    : TH1Bootstrap() {}

//______________________________________________________________________________
TH2Bootstrap::TH2Bootstrap(const TH2Bootstrap &h2b) : TH1Bootstrap(h2b) {}

//______________________________________________________________________________
TH2Bootstrap::TH2Bootstrap(const char *name, const char *title, Int_t nreplica,
                           BootstrapGenerator *boot)
    : TH1Bootstrap(name, title, nreplica, boot) {}

//______________________________________________________________________________
TH2Bootstrap::~TH2Bootstrap() {}

//______________________________________________________________________________
Int_t TH2Bootstrap::Fill(Double_t x, Double_t y, Double_t w, UInt_t RunNumber,
                         UInt_t EventNumber, UInt_t mc_channel_number) {
  if (fOwnGen) fGenerator->Generate(RunNumber, EventNumber, mc_channel_number);
  const Int_t *BSWeights = fGenerator->GetWeights();
  for (Int_t i = 0; i < fNReplica; ++i) {
    for (Int_t j = 0; j < BSWeights[i]; ++j) {
      static_cast<TH2 *>(GetReplica(i))->Fill(x, y, w);
    }
  }
  return static_cast<TH2 *>(GetNominal())->Fill(x, y, w);
}

//______________________________________________________________________________
Double_t TH2Bootstrap::GetBootstrapMean(Int_t binx, Int_t biny) const {
  return TH1Bootstrap::GetBootstrapMean(GetNominal()->GetBin(binx, biny));
}

//______________________________________________________________________________
Double_t TH2Bootstrap::GetBootstrapRMS(Int_t binx, Int_t biny) const {
  return TH1Bootstrap::GetBootstrapRMS(GetNominal()->GetBin(binx, biny));
}

//______________________________________________________________________________
Double_t TH2Bootstrap::GetBootstrapCorel(Int_t binx1, Int_t biny1, Int_t binx2,
                                         Int_t biny2) const {
  return TH1Bootstrap::GetBootstrapCorel(GetNominal()->GetBin(binx1, biny1),
                                         GetNominal()->GetBin(binx2, biny2));
}

//______________________________________________________________________________
Double_t TH2Bootstrap::GetBootstrapCorel(Int_t binx1, Int_t biny1,
                                         const TH1Bootstrap &h2, Int_t binx2,
                                         Int_t biny2) const {
  return TH1Bootstrap::GetBootstrapCorel(GetNominal()->GetBin(binx1, biny1), h2,
                                         GetNominal()->GetBin(binx2, biny2));
}

//______________________________________________________________________________
TH1DBootstrap *TH2Bootstrap::ProjectionX(const char *name, Int_t firstybin,
                                         Int_t lastybin, Option_t *option) {
  return DoProjection(true, name, firstybin, lastybin, option);
}
//______________________________________________________________________________
TH1DBootstrap *TH2Bootstrap::ProjectionY(const char *name, Int_t firstxbin,
                                         Int_t lastxbin, Option_t *option) {
  return DoProjection(false, name, firstxbin, lastxbin, option);
}
//______________________________________________________________________________
TH1DBootstrap *TH2Bootstrap::DoProjection(bool onX, const char *name,
                                          Int_t firstbin, Int_t lastbin,
                                          Option_t *option) {
  // create the nominal histogram
  TH1D *proj_nom = 0;
  if (onX)
    proj_nom = static_cast<TH2 *>(GetNominal())
                   ->ProjectionX("tmp_name", firstbin, lastbin, option);
  else
    proj_nom = static_cast<TH2 *>(GetNominal())
                   ->ProjectionY("tmp_name", firstbin, lastbin, option);
  proj_nom->SetDirectory(0);

  // create the replica histograms
  Int_t nreps = GetNReplica();
  TH1D **proj_reps = new TH1D *[nreps];
  TString repname = "";
  for (Int_t i = 0; i < nreps; ++i) {
    repname = TString::Format("tmp_name_%d", i);
    if (onX)
      proj_reps[i] =
          static_cast<TH2 *>(GetReplica(i))
              ->ProjectionX(repname.Data(), firstbin, lastbin, option);
    else
      proj_reps[i] =
          static_cast<TH2 *>(GetReplica(i))
              ->ProjectionY(repname.Data(), firstbin, lastbin, option);
    proj_reps[i]->SetDirectory(0);
  }

  TH1DBootstrap *proj = NULL;
  if (fOwnGen)
    proj = new TH1DBootstrap(name, name, proj_nom, proj_reps, nreps);
  else
    proj =
        new TH1DBootstrap(name, name, proj_nom, proj_reps, nreps, fGenerator);

  delete proj_nom;
  for (Int_t i = 0; i < nreps; ++i) delete proj_reps[i];
  delete[] proj_reps;

  return proj;
}

//______________________________________________________________________________
//            TH2DBootstrap methods
//______________________________________________________________________________

ClassImp(TH2DBootstrap)

    //______________________________________________________________________________
    TH2DBootstrap::TH2DBootstrap()
    : TH2Bootstrap(), fHist(NULL), fHistReplica(NULL) {}

//______________________________________________________________________________
TH2DBootstrap::TH2DBootstrap(const TH2DBootstrap &h2b) : TH2Bootstrap(h2b) {
  fHist = new TH2D(*h2b.fHist);

  if (fNReplica > 0) fHistReplica = new TH2D *[fNReplica];
  for (Int_t i = 0; i < fNReplica; ++i) {
    fHistReplica[i] = new TH2D(*h2b.fHistReplica[i]);
  }
}

//______________________________________________________________________________
TH2DBootstrap::TH2DBootstrap(const TH2D &h2d, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH2Bootstrap(h2d.GetName(), h2d.GetTitle(), nreplica, boot) {
  fHist = new TH2D(h2d);
  fHist->SetDirectory(0);

  if (fNReplica > 0) fHistReplica = new TH2D *[fNReplica];
  TString repname = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    fHistReplica[i] = new TH2D(h2d);
    repname = TString::Format("%s_rep%d", h2d.GetName(), i);
    fHistReplica[i]->SetName(repname.Data());
    fHistReplica[i]->SetDirectory(0);
  }
}

//______________________________________________________________________________
TH2DBootstrap::TH2DBootstrap(const char *name, const char *title, Int_t nxbins,
                             const Double_t *xbins, Int_t nybins,
                             const Double_t *ybins, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH2Bootstrap(name, title, nreplica, boot) {
  fHist = new TH2D(name, title, nxbins, xbins, nybins, ybins);
  fHist->SetDirectory(0);

  if (fNReplica > 0) fHistReplica = new TH2D *[fNReplica];
  TString repname = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    repname = TString::Format("%s_rep%d", name, i);
    fHistReplica[i] =
        new TH2D(repname.Data(), title, nxbins, xbins, nybins, ybins);
    fHistReplica[i]->SetDirectory(0);
  }
}

//______________________________________________________________________________
TH2DBootstrap::TH2DBootstrap(const char *name, const char *title, Int_t nxbins,
                             const Float_t *xbins, Int_t nybins,
                             const Float_t *ybins, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH2Bootstrap(name, title, nreplica, boot) {
  fHist = new TH2D(name, title, nxbins, xbins, nybins, ybins);
  fHist->SetDirectory(0);

  if (fNReplica > 0) fHistReplica = new TH2D *[fNReplica];
  TString repname = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    repname = TString::Format("%s_rep%d", name, i);
    fHistReplica[i] =
        new TH2D(repname.Data(), title, nxbins, xbins, nybins, ybins);
    fHistReplica[i]->SetDirectory(0);
  }
}

//______________________________________________________________________________
TH2DBootstrap::TH2DBootstrap(const char *name, const char *title, Int_t nxbins,
                             Double_t xlow, Double_t xup, Int_t nybins,
                             Double_t ylow, Double_t yup, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH2Bootstrap(name, title, nreplica, boot) {
  fHist = new TH2D(name, title, nxbins, xlow, xup, nybins, ylow, yup);
  fHist->SetDirectory(0);

  if (fNReplica > 0) fHistReplica = new TH2D *[fNReplica];
  TString repname = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    repname = TString::Format("%s_rep%d", name, i);
    fHistReplica[i] =
        new TH2D(repname.Data(), title, nxbins, xlow, xup, nybins, ylow, yup);
    fHistReplica[i]->SetDirectory(0);
  }
}

//______________________________________________________________________________
TH2DBootstrap::TH2DBootstrap(const char *name, const char *title, TH2D *nominal,
                             TH2D **replicas, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH2Bootstrap(name, title, nreplica, boot) {
  // set the nominal histogram to "nominal"
  if (nominal) {
    fHist = static_cast<TH2D *>(nominal->Clone());
    fHist->SetDirectory(0);
    fHist->SetName(name);
    fHist->SetTitle(title);
  } else {
    Error("TH2DBootstrap", "Pointer to the nominal histogram not provided");
  }

  // set the array of TH2D histograms to "replicas"
  if (fNReplica > 0 && replicas) {
    fHistReplica = new TH2D *[fNReplica];
    TString repname = "";
    for (Int_t i = 0; i < fNReplica; ++i) {
      fHistReplica[i] = static_cast<TH2D *>(replicas[i]->Clone());
      fHistReplica[i]->SetDirectory(0);
      repname = TString::Format("%s_rep%d", name, i);
      fHistReplica[i]->SetName(repname.Data());
      fHistReplica[i]->SetTitle(title);
    }
  } else if (fNReplica > 0 && !replicas) {
    Error("TH2DBootstrap", "Pointer to the array of replicas not provided");
  }
}

//______________________________________________________________________________
TH2DBootstrap::~TH2DBootstrap() {
  SafeDelete(fHist);
  for (Int_t i = 0; i < fNReplica; ++i) {
    SafeDelete(fHistReplica[i]);
  }
  if (fHistReplica) delete[] fHistReplica;
}

//______________________________________________________________________________
TH1 *TH2DBootstrap::GetNominal() { return fHist; }

//______________________________________________________________________________
const TH1 *TH2DBootstrap::GetNominal() const { return fHist; }

//______________________________________________________________________________
TH1 *TH2DBootstrap::GetReplica(Int_t i) {
  if (i >= fNReplica) {
    Error("GetReplica", "Cannot request replica %d out of %d", i, fNReplica);
    return NULL;
  }
  return fHistReplica[i];
}

//______________________________________________________________________________
const TH1 *TH2DBootstrap::GetReplica(Int_t i) const { return fHistReplica[i]; }

//______________________________________________________________________________
void TH2DBootstrap::SetNominal(const TH2D &h2) {
  if (!fHist)
    fHist = new TH2D(h2);
  else
    h2.Copy(*fHist);
}

//______________________________________________________________________________
void TH2DBootstrap::SetReplica(Int_t i, const TH2D &h2) {
  if (!fHistReplica) return;
  if (!fHistReplica[i])
    fHistReplica[i] = new TH2D(h2);
  else
    h2.Copy(*fHistReplica[i]);
}
