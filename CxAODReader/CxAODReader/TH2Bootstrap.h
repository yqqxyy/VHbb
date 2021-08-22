#ifndef TH2Bootstrap_h
#define TH2Bootstrap_h

#ifndef TH1Bootstrap_h
#include "CxAODReader/TH1Bootstrap.h"
#endif

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

class BootstrapGenerator;
class TH2D;

class TH2Bootstrap : public TH1Bootstrap {
 private:
  Int_t Fill(Double_t, Double_t, UInt_t, UInt_t, UInt_t) {
    return -1;
  }                                                       // May not use
  void Append(const TH1Bootstrap &) { return; }           // May not use for 2-D
  TMatrixD *GetCovarianceMatrix() const { return NULL; }  // May not use for 2-D
  TMatrixD *GetCorrelationMatrix() const {
    return NULL;
  }  // May not use for 2-D

 protected:
  TH2Bootstrap();
  TH2Bootstrap(const TH2Bootstrap &h2b);
  TH2Bootstrap(const char *name, const char *title, Int_t nreplica,
               BootstrapGenerator *boot = NULL);
  virtual ~TH2Bootstrap();

 public:
  virtual Int_t Fill(Double_t x, Double_t y, Double_t w = 1.0,
                     UInt_t RunNumber = 0, UInt_t EventNumber = 0,
                     UInt_t mc_channel_number = 0);

  using TH1Bootstrap::GetBootstrapCorel;
  using TH1Bootstrap::GetBootstrapMean;
  using TH1Bootstrap::GetBootstrapRMS;
  virtual Double_t GetBootstrapMean(Int_t binx, Int_t biny) const;
  virtual Double_t GetBootstrapRMS(Int_t binx, Int_t biny) const;
  virtual Double_t GetBootstrapCorel(Int_t binx1, Int_t biny1, Int_t binx2,
                                     Int_t biny2) const;
  virtual Double_t GetBootstrapCorel(Int_t binx1, Int_t biny1,
                                     const TH1Bootstrap &h2, Int_t binx2,
                                     Int_t biny2) const;

  virtual TH1DBootstrap *ProjectionX(const char *name = "_px",
                                     Int_t firstybin = 0, Int_t lastybin = -1,
                                     Option_t *option = "");
  virtual TH1DBootstrap *ProjectionY(const char *name = "_py",
                                     Int_t firstxbin = 0, Int_t lastxbin = -1,
                                     Option_t *option = "");
  virtual TH1DBootstrap *DoProjection(bool onX, const char *name,
                                      Int_t firstbin, Int_t lastbin,
                                      Option_t *option);

  ClassDef(TH2Bootstrap, 3)
};

//______________________________________________________________________________
class TH2DBootstrap : public TH2Bootstrap {
 protected:
  TH2D *fHist;
  TH2D **fHistReplica;  //[fNReplica]

 public:
  TH2DBootstrap();
  TH2DBootstrap(const TH2DBootstrap &h2b);
  TH2DBootstrap(const TH2D &h2d, Int_t nreplica,
                BootstrapGenerator *boot = NULL);
  TH2DBootstrap(const char *name, const char *title, Int_t nxbins,
                Double_t xlow, Double_t xup, Int_t nybins, Double_t ylow,
                Double_t yup, Int_t nreplica, BootstrapGenerator *boot = NULL);
  TH2DBootstrap(const char *name, const char *title, Int_t nxbins,
                const Double_t *xbins, Int_t nybins, const Double_t *ybins,
                Int_t nreplica, BootstrapGenerator *boot = NULL);
  TH2DBootstrap(const char *name, const char *title, Int_t nxbins,
                const Float_t *xbins, Int_t nybins, const Float_t *ybins,
                Int_t nreplica, BootstrapGenerator *boot = NULL);
  TH2DBootstrap(const char *name, const char *title, TH2D *nominal,
                TH2D **replicas, Int_t nreplica,
                BootstrapGenerator *boot = NULL);
  virtual ~TH2DBootstrap();

  virtual TH1 *GetNominal();
  virtual TH1 *GetReplica(Int_t i);

  virtual const TH1 *GetNominal() const;
  virtual const TH1 *GetReplica(Int_t i) const;

  virtual void SetNominal(const TH2D &h2);
  virtual void SetReplica(Int_t i, const TH2D &h2);

  ClassDef(TH2DBootstrap, 4)
};

#endif
