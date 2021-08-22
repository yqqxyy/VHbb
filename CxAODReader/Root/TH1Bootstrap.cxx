#ifndef _BootstrapGenerator
#include "CxAODReader/BootstrapGenerator.h"
#endif

#ifndef TH1Bootstrap_h
#include "CxAODReader/TH1Bootstrap.h"
#endif

#ifndef ROOT_TBrowser
#include "TBrowser.h"
#endif

#ifndef ROOT_TCollection
#include "TCollection.h"
#endif

#ifndef ROOT_TH1D
#include "TH1D.h"
#endif

#ifndef ROOT_TList
#include "TList.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#include <cmath>

ClassImp(TH1Bootstrap)

    //______________________________________________________________________________
    TH1Bootstrap::TH1Bootstrap()
    : TNamed(), fNReplica(0), fGenerator(NULL), fOwnGen(kTRUE) {}

//______________________________________________________________________________
TH1Bootstrap::TH1Bootstrap(const TH1Bootstrap &h1b)
    : TNamed(h1b), fNReplica(h1b.fNReplica), fOwnGen(h1b.fOwnGen) {
  if (fOwnGen) {
    fGenerator = new BootstrapGenerator(*h1b.fGenerator);
  } else {
    fGenerator = h1b.fGenerator;
  }
}

//______________________________________________________________________________
TH1Bootstrap::TH1Bootstrap(const char *name, const char *title, Int_t nreplica,
                           BootstrapGenerator *boot)
    : TNamed(name, title), fNReplica(nreplica), fOwnGen(kFALSE) {
  if (boot) {
    fGenerator = boot;
    if (fGenerator->GetSize() < fNReplica) fGenerator->Set(fNReplica);
  } else {
    fGenerator = new BootstrapGenerator("Generator", "Generator", nreplica);
    fOwnGen = kTRUE;
  }
}

//______________________________________________________________________________
TH1Bootstrap::~TH1Bootstrap() {
  if (fOwnGen) SafeDelete(fGenerator);
}

//______________________________________________________________________________
void TH1Bootstrap::Browse(TBrowser *b) {
  b->Add(this->GetNominal(), "Nominal");
  TString name = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    name = TString::Format("Replica%d", i);
    b->Add(this->GetReplica(i), name.Data());
  }
}

//______________________________________________________________________________
void TH1Bootstrap::Append(const TH1 &hext, TH1 *hbase) {
  Int_t nbinsbase = hbase->GetNbinsX();
  Int_t nbinsext = hext.GetNbinsX();
  Int_t nbinstot = nbinsbase + nbinsext;

  hbase->SetBins(nbinstot, 1, nbinstot + 1);

  hbase->SetBinContent(0, 0);
  for (Int_t i = 1; i <= nbinsext; ++i)
    hbase->SetBinContent(nbinsbase + i, hext.GetBinContent(i));
  hbase->SetBinContent(nbinstot + 1, 0);
}

//______________________________________________________________________________
Int_t TH1Bootstrap::Fill(Double_t x, Double_t w, UInt_t RunNumber,
                         UInt_t EventNumber, UInt_t mc_channel_number) {
  if (fOwnGen)
    fGenerator->Generate(w, RunNumber, EventNumber, mc_channel_number);
  const Int_t *BSWeights = fGenerator->GetWeights();
  for (Int_t i = 0; i < fNReplica; ++i) {
    for (Int_t j = 0; j < BSWeights[i]; ++j) {
      GetReplica(i)->Fill(x, w);
    }
  }
  return GetNominal()->Fill(x, w);
}

//______________________________________________________________________________
void TH1Bootstrap::AddBinContent(Int_t bin, Double_t w, UInt_t RunNumber,
                                 UInt_t EventNumber, UInt_t mc_channel_number) {
  if (fOwnGen)
    fGenerator->Generate(w, RunNumber, EventNumber, mc_channel_number);
  const Int_t *BSWeights = fGenerator->GetWeights();
  for (Int_t i = 0; i < fNReplica; ++i) {
    for (Int_t j = 0; j < BSWeights[i]; ++j) {
      GetReplica(i)->AddBinContent(bin, w);
    }
  }
  GetNominal()->AddBinContent(bin, w);
}

//______________________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(5, 34, 0)
Bool_t TH1Bootstrap::Add(const TH1Bootstrap *h1, Double_t c1) {
  if (!IsCompatible(*h1)) {
    Error("Add", "Cannot add for incompatible bootstraps");
    return kFALSE;
  }
  for (Int_t i = 0; i < fNReplica; ++i) {
    GetReplica(i)->Add(h1->GetReplica(i), c1);
  }
  return GetNominal()->Add(h1->GetNominal(), c1);
}
#else
void TH1Bootstrap::Add(const TH1Bootstrap *h1, Double_t c1) {
  if (!IsCompatible(*h1)) {
    Error("Add", "Cannot add for incompatible bootstraps");
  }
  for (Int_t i = 0; i < fNReplica; ++i) {
    GetReplica(i)->Add(h1->GetReplica(i), c1);
  }
  GetNominal()->Add(h1->GetNominal(), c1);
}
#endif

//______________________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(5, 34, 0)
Bool_t TH1Bootstrap::Multiply(const TH1Bootstrap *h1) {
  if (!IsCompatible(*h1)) {
    Error("Multiply", "Cannot multiply for incompatible bootstraps");
    return kFALSE;
  }
  for (Int_t i = 0; i < fNReplica; ++i) {
    GetReplica(i)->Multiply(h1->GetReplica(i));
  }
  return GetNominal()->Multiply(h1->GetNominal());
}
#else
void TH1Bootstrap::Multiply(const TH1Bootstrap *h1) {
  if (!IsCompatible(*h1)) {
    Error("Multiply", "Cannot multiply for incompatible bootstraps");
  }
  for (Int_t i = 0; i < fNReplica; ++i) {
    GetReplica(i)->Multiply(h1->GetReplica(i));
  }
  GetNominal()->Multiply(h1->GetNominal());
}
#endif

//______________________________________________________________________________
#if ROOT_VERSION_CODE >= ROOT_VERSION(5, 34, 0)
Bool_t TH1Bootstrap::Divide(const TH1Bootstrap *h1) {
  if (!IsCompatible(*h1)) {
    Error("Divide", "Cannot divide for incompatible bootstraps");
    return kFALSE;
  }
  for (Int_t i = 0; i < fNReplica; ++i) {
    GetReplica(i)->Divide(h1->GetReplica(i));
  }
  return GetNominal()->Divide(h1->GetNominal());
}
#else
void TH1Bootstrap::Divide(const TH1Bootstrap *h1) {
  if (!IsCompatible(*h1)) {
    Error("Divide", "Cannot divide for incompatible bootstraps");
  }
  for (Int_t i = 0; i < fNReplica; ++i) {
    GetReplica(i)->Divide(h1->GetReplica(i));
  }
  GetNominal()->Divide(h1->GetNominal());
}
#endif

//______________________________________________________________________________
Double_t TH1Bootstrap::GetBootstrapMean(Int_t bin) const {
  Double_t mean = 0.0;
  if (fNReplica > 0) {
    for (Int_t i = 0; i < fNReplica; ++i) {
      mean += GetReplica(i)->GetBinContent(bin);
    }
    mean /= fNReplica;
  }
  return mean;
}

//______________________________________________________________________________
Double_t TH1Bootstrap::GetBootstrapRMS(Int_t bin) const {
  Double_t mean = GetBootstrapMean(bin);
  Double_t rms = 0.0, val = 0.0;
  if (fNReplica > 0) {
    for (Int_t i = 0; i < fNReplica; ++i) {
      val = GetReplica(i)->GetBinContent(bin);
      rms += val * val;
    }
    rms = sqrt(fabs(rms / fNReplica - mean * mean));
  }
  return rms;
}

//______________________________________________________________________________
Double_t TH1Bootstrap::GetBootstrapCorel(Int_t bin1, Int_t bin2) const {
  Double_t mean1 = GetBootstrapMean(bin1);
  Double_t mean2 = GetBootstrapMean(bin2);
  Double_t rms1 = GetBootstrapRMS(bin1);
  Double_t rms2 = GetBootstrapRMS(bin2);
  Double_t corel = 0.0, val1 = 0.0, val2 = 0.0;
  if (fNReplica > 0 && rms1 > 0 && rms2 > 0) {
    for (Int_t i = 0; i < fNReplica; ++i) {
      val1 = GetReplica(i)->GetBinContent(bin1);
      val2 = GetReplica(i)->GetBinContent(bin2);
      corel += (val1 - mean1) * (val2 - mean2);
    }
    corel = corel / fNReplica / (rms1 * rms2);
  }
  return corel;
}

//______________________________________________________________________________
Double_t TH1Bootstrap::GetBootstrapCorel(Int_t bin1, const TH1Bootstrap &h1b,
                                         Int_t bin2) const {
  //if (!IsCompatible(h1b)) {
  if (fNReplica != h1b.GetNReplica()) {
    Error("GetBootstrapCorel", "Cannot calculate for incompatible bootstraps");
    return 0.0;
  }
  Double_t mean1 = GetBootstrapMean(bin1);
  Double_t mean2 = h1b.GetBootstrapMean(bin2);
  Double_t rms1 = GetBootstrapRMS(bin1);
  Double_t rms2 = h1b.GetBootstrapRMS(bin2);
  Double_t corel = 0.0, val1 = 0.0, val2 = 0.0;
  if (fNReplica > 0 && rms1 > 0 && rms2 > 0) {
    for (Int_t i = 0; i < fNReplica; ++i) {
      val1 = GetReplica(i)->GetBinContent(bin1);
      val2 = h1b.GetReplica(i)->GetBinContent(bin2);
      corel += (val1 - mean1) * (val2 - mean2);
    }
    corel = corel / fNReplica / (rms1 * rms2);
  }
  return corel;
}

//______________________________________________________________________________
TMatrixD *TH1Bootstrap::GetCovarianceMatrix() const {
  TH1 *havg = GetBootstrapResult("mean");
  Int_t nbins = havg->GetNbinsX();

  TMatrixD *cov = new TMatrixD(nbins, nbins);

  // SVD, IDS1, IDS2 covariance matrix
  for (Int_t i = 0; i < nbins; i++) {
    for (Int_t j = i; j < nbins; j++) {
      (*cov)[i][j] = 0.;
      for (Int_t toy = 0; toy < fNReplica; toy++) {
        (*cov)[i][j] += (GetReplica(toy)->GetBinContent(i + 1) -
                         havg->GetBinContent(i + 1)) *
                        (GetReplica(toy)->GetBinContent(j + 1) -
                         havg->GetBinContent(j + 1)) /
                        fNReplica;
      }
      (*cov)[j][i] = (*cov)[i][j];
    }
  }
  delete havg;
  return cov;
}

//______________________________________________________________________________
TMatrixD *TH1Bootstrap::GetCorrelationMatrix() const {
  TMatrixD *cov = GetCovarianceMatrix();
  Int_t nbins = cov->GetNcols();
  TVectorD diag(nbins);

  for (Int_t i = 0; i < nbins; ++i) {
    diag[i] = (*cov)[i][i];
  }

  Double_t norm = 0.0;
  for (Int_t i = 0; i < nbins; ++i) {
    for (Int_t j = 0; j <= i; ++j) {
      norm = diag[i] * diag[j];
      (*cov)[i][j] = (norm > 0) ? (*cov)[i][j] / std::sqrt(norm) : 0;
      if (i != j) (*cov)[j][i] = (*cov)[i][j];
    }
  }

  return cov;
}

//______________________________________________________________________________
void TH1Bootstrap::SetValBootstrapMean() {
  Int_t N = GetSize();
  for (Int_t i = 0; i <= N + 1; ++i) {
    GetNominal()->SetBinContent(i, GetBootstrapMean(i));
  }
}

//______________________________________________________________________________
void TH1Bootstrap::SetErrBootstrapRMS() {
  Int_t N = GetSize();
  for (Int_t i = 0; i < N + 1; ++i) {
    GetNominal()->SetBinError(i, GetBootstrapRMS(i));
  }
}

//______________________________________________________________________________
TH1 *TH1Bootstrap::GetBootstrapResult(Option_t *option) const {
  // Returns a new histogram with bootstrap results. The name is
  // set to the nominal histogram name suffixed with "_bs".
  // The user is responsible for deleting the new histogram.
  //
  // If option is empty (default) or contains "mean" and "rms" the bin
  // contents is set to the bootstrap mean and the bin error is set to
  // the bootstrap rms.
  //
  // If option contains "mean" and not "rms" the bin contents is set to
  // the bootstrap mean and the bin error is taken from the nominal
  // histogram.
  //
  // If option contains "rms" and not "mean" the bin _contents_ is set to
  // the bootstrap rms and the bin _error_ is set to 0.
  //
  // If option is not empty and does not contain "mean" and/or "rms" a
  // warning is issued and an empty histogram is returned.

  const Int_t N = GetSize();

  const TString opt(option);

  const Bool_t mean = (opt != "") ? opt.Contains("mean") : kTRUE;
  const Bool_t rms = (opt != "") ? opt.Contains("rms") : kTRUE;

  TH1 *result = static_cast<TH1 *>(GetNominal()->Clone());
  result->Reset();
  result->SetName(GetNominal()->GetName() + TString("_bs"));

  if (!(mean || rms)) {
    Warning("GetBootstrapResult", "Unknown option: \"%s\"", option);
    return result;
  }

  for (Int_t i = 0; i < N + 1; ++i) {
    if (mean) {
      result->SetBinContent(i, GetBootstrapMean(i));
      if (rms) {
        result->SetBinError(i, GetBootstrapRMS(i));
      }
    } else if (rms) {
      result->SetBinContent(i, GetBootstrapRMS(i));
    }
  }

  return result;
}

Int_t TH1Bootstrap::GetSize() const {
  Int_t nx = GetNominal()->GetNbinsX();
  if (nx > 1) nx += 2;
  Int_t ny = GetNominal()->GetNbinsY();
  if (ny > 1) ny += 2;
  Int_t nz = GetNominal()->GetNbinsZ();
  if (nz > 1) nz += 2;
  return nx * ny * nz;
}

//______________________________________________________________________________
Bool_t TH1Bootstrap::IsCompatible(const TH1Bootstrap &h1b) const {
  if (GetSize() != h1b.GetSize()) return kFALSE;
  if (fNReplica != h1b.GetNReplica()) return kFALSE;
  return kTRUE;
}

//______________________________________________________________________________
void TH1Bootstrap::Scale(Double_t c1, Option_t *option) {
  for (Int_t i = 0; i < fNReplica; ++i) {
    GetReplica(i)->Scale(c1, option);
  }
  GetNominal()->Scale(c1, option);
}

//______________________________________________________________________________
Long64_t TH1Bootstrap::Merge(TCollection *li) {
  if (!li) return 0;
  if (li->IsEmpty()) return (Long64_t)GetNominal()->GetEntries();

  TList inlist;
  inlist.AddAll(li);
  TIter next(&inlist);

  // Check if all are compatible first
  while (TH1Bootstrap *h1b = (TH1Bootstrap *)next()) {
    if (!IsCompatible(*h1b)) {
      Error("Merge", "Cannot merge incompatible bootstraps");
      return -1;
    }
  }

  next.Reset();

  // Merge all bootstraps
  while (TH1Bootstrap *h1b = (TH1Bootstrap *)next()) {
    GetNominal()->Add(h1b->GetNominal());
    for (Int_t i = 0; i < fNReplica; ++i) {
      GetReplica(i)->Add(h1b->GetReplica(i));
    }
  }

  return (Long64_t)GetNominal()->GetEntries();
}

//______________________________________________________________________________
void TH1Bootstrap::SetGenerator(BootstrapGenerator *gen) {
  if (fOwnGen && fGenerator) delete fGenerator;
  if (gen && fOwnGen) {
    fGenerator = new BootstrapGenerator(*gen);
  } else if (gen && !fOwnGen) {
    fGenerator = gen;
  } else {
    fGenerator = new BootstrapGenerator("Generator", "Generator", fNReplica);
  }
}

//______________________________________________________________________________
void TH1Bootstrap::Append(const TH1Bootstrap &hext) {
  if (hext.GetNReplica() != fNReplica) {
    Error("Append",
          "The passed TH1Bootstrap has a different number of replicas.");
    Error("Append", "Appending aborted.");
    return;
  }

  Append(*hext.GetNominal(), GetNominal());
  for (Int_t i = 0; i < fNReplica; ++i) {
    Append(*hext.GetReplica(i), GetReplica(i));
  }
}

//______________________________________________________________________________
//            TH1DBootstrap methods
//______________________________________________________________________________

ClassImp(TH1DBootstrap)

    //______________________________________________________________________________
    TH1DBootstrap::TH1DBootstrap()
    : TH1Bootstrap(), fHist(NULL), fHistReplica(NULL) {}

//______________________________________________________________________________
TH1DBootstrap::TH1DBootstrap(const TH1DBootstrap &h1b) : TH1Bootstrap(h1b) {
  fHist = new TH1D(*h1b.fHist);

  if (fNReplica > 0) fHistReplica = new TH1D *[fNReplica];
  for (Int_t i = 0; i < fNReplica; ++i) {
    fHistReplica[i] = new TH1D(*h1b.fHistReplica[i]);
  }
}

//______________________________________________________________________________
TH1DBootstrap::TH1DBootstrap(const TH1D &h1d, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH1Bootstrap(h1d.GetName(), h1d.GetTitle(), nreplica, boot) {
  fHist = new TH1D(h1d);
  fHist->SetDirectory(0);

  if (fNReplica > 0) fHistReplica = new TH1D *[fNReplica];
  TString repname = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    fHistReplica[i] = new TH1D(h1d);
    repname = TString::Format("%s_rep%d", h1d.GetName(), i);
    fHistReplica[i]->SetName(repname.Data());
    fHistReplica[i]->SetDirectory(0);
  }
}

//______________________________________________________________________________
TH1DBootstrap::TH1DBootstrap(const char *name, const char *title, Int_t nxbins,
                             const Float_t *xbins, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH1Bootstrap(name, title, nreplica, boot) {
  fHist = new TH1D(name, title, nxbins, xbins);
  fHist->SetDirectory(0);

  if (fNReplica > 0) fHistReplica = new TH1D *[fNReplica];
  TString repname = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    repname = TString::Format("%s_rep%d", name, i);
    fHistReplica[i] = new TH1D(repname.Data(), title, nxbins, xbins);
    fHistReplica[i]->SetDirectory(0);
  }
}

//______________________________________________________________________________
TH1DBootstrap::TH1DBootstrap(const char *name, const char *title, Int_t nxbins,
                             const Double_t *xbins, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH1Bootstrap(name, title, nreplica, boot) {
  fHist = new TH1D(name, title, nxbins, xbins);
  fHist->SetDirectory(0);

  if (fNReplica > 0) fHistReplica = new TH1D *[fNReplica];
  TString repname = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    repname = TString::Format("%s_rep%d", name, i);
    fHistReplica[i] = new TH1D(repname.Data(), title, nxbins, xbins);
    fHistReplica[i]->SetDirectory(0);
  }
}

//______________________________________________________________________________
TH1DBootstrap::TH1DBootstrap(const char *name, const char *title, Int_t nxbins,
                             Double_t xlow, Double_t xup, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH1Bootstrap(name, title, nreplica, boot) {
  fHist = new TH1D(name, title, nxbins, xlow, xup);
  fHist->SetDirectory(0);

  if (fNReplica > 0) fHistReplica = new TH1D *[fNReplica];
  TString repname = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    repname = TString::Format("%s_rep%d", name, i);
    fHistReplica[i] = new TH1D(repname.Data(), title, nxbins, xlow, xup);
    fHistReplica[i]->SetDirectory(0);
  }
}

//______________________________________________________________________________
TH1DBootstrap::TH1DBootstrap(const char *name, const char *title, TH1D *nominal,
                             TH1D **replicas, Int_t nreplica,
                             BootstrapGenerator *boot)
    : TH1Bootstrap(name, title, nreplica, boot) {
  // set the nominal histogram to "nominal"
  if (nominal) {
    fHist = static_cast<TH1D *>(nominal->Clone());
    fHist->SetDirectory(0);
    fHist->SetName(name);
    fHist->SetTitle(title);
  } else {
    Error("TH1DBootstrap", "Pointer to the nominal histogram not provided");
  }

  // set the array of TH1D histograms to "replicas"
  if (fNReplica > 0 && replicas) {
    fHistReplica = new TH1D *[fNReplica];
    TString repname = "";
    for (Int_t i = 0; i < fNReplica; ++i) {
      fHistReplica[i] = static_cast<TH1D *>(replicas[i]->Clone());
      fHistReplica[i]->SetDirectory(0);
      repname = TString::Format("%s_rep%d", name, i);
      fHistReplica[i]->SetName(repname.Data());
      fHistReplica[i]->SetTitle(title);
    }
  } else if (fNReplica > 0 && !replicas) {
    Error("TH1DBootstrap", "Pointer to the array of replicas not provided");
  }
}

//______________________________________________________________________________
TH1DBootstrap::~TH1DBootstrap() {
  SafeDelete(fHist);
  for (Int_t i = 0; i < fNReplica; ++i) {
    SafeDelete(fHistReplica[i]);
  }
  if (fHistReplica) delete[] fHistReplica;
}

//______________________________________________________________________________
TH1DBootstrap *TH1DBootstrap::Rebin(Int_t ngroup, const char *newname,
                                    const Double_t *xbins) {
  // create a clone of the old histogram if newname is specified
  TH1DBootstrap *hnew = this;
  if ((newname && strlen(newname) > 0) || xbins) {
    hnew = (TH1DBootstrap *)Clone(newname);
    hnew->SetGenerator(fGenerator);
  }

  hnew->SetNominal(
      *static_cast<TH1D *>(GetNominal()->Rebin(ngroup, newname, xbins)));
  hnew->GetNominal()->SetDirectory(0);
  TString repname = "";
  for (Int_t i = 0; i < fNReplica; ++i) {
    repname = TString::Format("%s_rep%d", newname, i);
    hnew->SetReplica(i, *static_cast<TH1D *>(GetReplica(i)->Rebin(
                            ngroup, repname.Data(), xbins)));
    hnew->GetReplica(i)->SetDirectory(0);
  }

  return hnew;
}

//______________________________________________________________________________
TH1 *TH1DBootstrap::GetNominal() { return fHist; }

//______________________________________________________________________________
const TH1 *TH1DBootstrap::GetNominal() const { return fHist; }

//______________________________________________________________________________
TH1 *TH1DBootstrap::GetReplica(Int_t i) {
  if (i >= fNReplica) {
    Error("GetReplica", "Cannot request replica %d out of %d", i, fNReplica);
    return NULL;
  }
  return fHistReplica[i];
}

//______________________________________________________________________________
const TH1 *TH1DBootstrap::GetReplica(Int_t i) const { return fHistReplica[i]; }

//______________________________________________________________________________
void TH1DBootstrap::SetNominal(const TH1D &h1) {
  if (!fHist) {
    fHist = new TH1D(h1);
  } else {
    h1.Copy(*fHist);
  }
  fHist->SetDirectory(0);
}

//______________________________________________________________________________
void TH1DBootstrap::SetReplica(Int_t i, const TH1D &h1) {
  if (!fHistReplica || i >= fNReplica) return;
  if (!fHistReplica[i])
    fHistReplica[i] = new TH1D(h1);
  else
    h1.Copy(*fHistReplica[i]);
  fHistReplica[i]->SetDirectory(0);
}
