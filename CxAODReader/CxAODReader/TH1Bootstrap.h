#ifndef TH1Bootstrap_h
#define TH1Bootstrap_h

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

#ifndef ROOT_TMatrixD
#include "TMatrixD.h"
#endif

#ifndef ROOT_TVectorD
#include "TVectorD.h"
#endif

class BootstrapGenerator;
class TBrowser;
class TCollection;
class TH1;
class TH1D;

class TH1Bootstrap : public TNamed {
 protected:
  Int_t fNReplica;
  BootstrapGenerator *fGenerator;
  Bool_t fOwnGen;

 protected:
  TH1Bootstrap();
  TH1Bootstrap(const TH1Bootstrap &h1b);
  TH1Bootstrap(const char *name, const char *title, Int_t nreplica,
               BootstrapGenerator *boot = NULL);
  virtual ~TH1Bootstrap();

  static void Append(const TH1 &extension, TH1 *base);

 public:
  virtual TH1 *GetNominal() = 0;
  virtual TH1 *GetReplica(Int_t i) = 0;

  virtual const TH1 *GetNominal() const = 0;
  virtual const TH1 *GetReplica(Int_t i) const = 0;

  virtual void Browse(TBrowser *b);

  virtual Int_t Fill(Double_t x, Double_t w, UInt_t RunNumber = 0,
                     UInt_t EventNumber = 0, UInt_t mc_channel_number = 0);
  virtual void AddBinContent(Int_t bin, Double_t w, UInt_t RunNumber = 0,
                             UInt_t EventNumber = 0,
                             UInt_t mc_channel_number = 0);
  virtual Int_t GetSize() const;
  virtual Int_t GetNReplica() const { return fNReplica; }
  virtual Long64_t Merge(TCollection *li);
  virtual Bool_t IsCompatible(const TH1Bootstrap &h1b) const;
  virtual void Scale(Double_t c1 = 1, Option_t *option = "");

//#if ROOT_VERSION_CODE >= ROOT_VERSION(5,34,0)
#if ROOT_VERSION_CODE >= 336384
  virtual Bool_t Divide(const TH1Bootstrap *h1);
  virtual Bool_t Add(const TH1Bootstrap *h1, Double_t c1 = 1);
  virtual Bool_t Multiply(const TH1Bootstrap *h1);
#else
  virtual void Divide(const TH1Bootstrap *h1);
  virtual void Add(const TH1Bootstrap *h1, Double_t c1 = 1);
  virtual void Multiply(const TH1Bootstrap *h1);
#endif

  // Extra helper functions:
  virtual Double_t GetBootstrapMean(Int_t bin) const;
  virtual Double_t GetBootstrapRMS(Int_t bin) const;
  virtual Double_t GetBootstrapCorel(Int_t bin1, Int_t bin2) const;
  virtual Double_t GetBootstrapCorel(Int_t bin1, const TH1Bootstrap &h2,
                                     Int_t bin2) const;

  virtual TMatrixD *GetCovarianceMatrix() const;
  virtual TMatrixD *GetCorrelationMatrix() const;

  virtual void SetValBootstrapMean();  // reset bin contents to bootstrap mean
  virtual void SetErrBootstrapRMS();   // reset bin contents to bootstrap rms

  virtual TH1 *GetBootstrapResult(Option_t *option = "") const;

  virtual void SetGenerator(BootstrapGenerator *gen = NULL);

  virtual void Append(const TH1Bootstrap &hext);

  ClassDef(TH1Bootstrap, 7)
};

//______________________________________________________________________________
class TH1DBootstrap : public TH1Bootstrap {
 protected:
  TH1D *fHist;
  TH1D **fHistReplica;  //[fNReplica]

 public:
  TH1DBootstrap();
  TH1DBootstrap(const TH1DBootstrap &h1b);
  TH1DBootstrap(const TH1D &h1d, Int_t nreplica,
                BootstrapGenerator *boot = NULL);
  TH1DBootstrap(const char *name, const char *title, Int_t nxbins,
                Double_t xlow, Double_t xup, Int_t nreplica,
                BootstrapGenerator *boot = NULL);
  TH1DBootstrap(const char *name, const char *title, Int_t nxbins,
                const Float_t *xbins, Int_t nreplica,
                BootstrapGenerator *boot = NULL);
  TH1DBootstrap(const char *name, const char *title, Int_t nxbins,
                const Double_t *xbins, Int_t nreplica,
                BootstrapGenerator *boot = NULL);
  TH1DBootstrap(const char *name, const char *title, TH1D *nominal,
                TH1D **replicas, Int_t nreplica,
                BootstrapGenerator *boot = NULL);
  virtual ~TH1DBootstrap();

  virtual TH1DBootstrap *Rebin(Int_t ngroup = 2, const char *newname = "",
                               const Double_t *xbins = NULL);

  virtual TH1 *GetNominal();
  virtual TH1 *GetReplica(Int_t i);

  virtual const TH1 *GetNominal() const;
  virtual const TH1 *GetReplica(Int_t i) const;

  virtual void SetNominal(const TH1D &h1);
  virtual void SetReplica(Int_t i, const TH1D &h1);

  ClassDef(TH1DBootstrap, 5)
};

#endif
