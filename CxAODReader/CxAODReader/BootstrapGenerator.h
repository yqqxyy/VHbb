#ifndef BootstrapGenerator_h
#define BootstrapGenerator_h

//
// Generate bootstrap random numbers
//

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TArray
#include "TArrayI.h"
#endif

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

class StochasticLib2;

class BootstrapGenerator : public TNamed, public TArrayI {
 private:
  Int_t fNReplica;  // Number of replicas to store
  Int_t fSeeds[3];

  StochasticLib2 *fStoch;  //!

 public:
  BootstrapGenerator();
  BootstrapGenerator(const char *name, const char *title, Int_t nrep,
                     Int_t seed = 65539);
  BootstrapGenerator(const BootstrapGenerator &bg);
  ~BootstrapGenerator();

  void Generate(Float_t eventweight, UInt_t run = 0, UInt_t event = 0,
                UInt_t mc_channel_number = 0);
  const Int_t *GetWeights() const { return fArray; }
  Int_t GetWeight(Int_t iweight) const { return fArray[iweight]; }
  Int_t GetNReplica() const { return fNReplica; }
  void Init(Int_t nrep, Int_t seed = 65539);
  virtual void Set(Int_t nrep);

  ClassDef(BootstrapGenerator, 3)
};

#endif
