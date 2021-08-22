#include "CxAODReader/BootstrapGenerator.h"

#ifndef STOCC_H
#include "CxAODReader/stocc.h"
#endif

// Initialise BootstrapGenerator with NRep replicas and starting SeeD
// Note that the SeeD should be normally reset for each event/run number.

ClassImp(BootstrapGenerator)

    BootstrapGenerator::BootstrapGenerator()
    : TNamed(), TArrayI(), fNReplica(0), fStoch(NULL) {}

BootstrapGenerator::BootstrapGenerator(const char *name, const char *title,
                                       Int_t nrep, Int_t seed)
    : TNamed(name, title), TArrayI(nrep), fNReplica(nrep) {
  fStoch = new StochasticLib2(seed);
}

BootstrapGenerator::BootstrapGenerator(const BootstrapGenerator &bg)
    : TNamed(bg.GetName(), bg.GetTitle()),
      TArrayI(bg.fNReplica),
      fNReplica(bg.fNReplica) {
  fStoch = new StochasticLib2(*bg.fStoch);
}

BootstrapGenerator::~BootstrapGenerator() { SafeDelete(fStoch); }

void BootstrapGenerator::Set(Int_t nrep) {
  fNReplica = nrep;
  TArrayI::Set(fNReplica);
}

void BootstrapGenerator::Init(Int_t nrep, Int_t seed) {
  Set(nrep);
  if (!fStoch)
    fStoch = new StochasticLib2(seed);
  else
    fStoch->RandomInit(seed);
}

void BootstrapGenerator::Generate(Float_t eventweight, UInt_t run, UInt_t event,
                                  UInt_t mc_channel_number) {
  // Generate Poisson-distributed random numbers. Set seed based on run/event #

  if (!fStoch) Error("Generate", "Not initialized properly, exiting!");
  if ((run != 0 || event != 0) && fNReplica > 0) {
    fSeeds[0] = run;
    fSeeds[1] = event;
    fSeeds[2] = mc_channel_number;
    fStoch->RandomInitByArray(fSeeds, 3);
  }

  for (Int_t i = 0; i < fNReplica; ++i) {
    //      fArray[i] = fStoch->Poisson(1.0);
    fArray[i] = fStoch->Poisson(fabs(eventweight));
  }
}
