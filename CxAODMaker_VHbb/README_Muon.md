How can we find out what is the object selection for `isWHSignal` electron or muon? It is defined in this package. An example below about the muon, and the same thing can be done for an electron too. 

I go to the code and search for that string:
```
-bash-4.1$ pwd
/data06/abuzatu/code/CxAODFramework_branch_master_21.2.46_1/source
-bash-4.1$ find . -name "*.cxx" | xargs grep -i isWHSignal
./CxAODMaker/Root/METHandler.cxx:
////Props::isWHSignalElectron.get(ele, passSel);
./CxAODMaker_VHbb/Root/ElectronHandler_VHbb.cxx:  Props::isWHSignalElectron.set(electron, passSel);
./CxAODMaker_VHbb/Root/ElectronHandler_VHbb.cxx:    Props::isWHSignalElectron.copy(inElectron, outElectron);
./CxAODMaker_VHbb/Root/MuonHandler_VHbb.cxx:  Props::isWHSignalMuon.set(muon, passSel);
./CxAODMaker_VHbb/Root/MuonHandler_VHbb.cxx:    Props::isWHSignalMuon.copy(inMuon, outMuon);
./CxAODReader_VHbb/Root/AnalysisReader_VHbb1Lep.cxx:    if (Props::isWHSignalElectron.get(el) == 1) nSigEl = 1;
./CxAODReader_VHbb/Root/AnalysisReader_VHbb1Lep.cxx:    if (Props::isWHSignalMuon.get(mu) == 1) nSigMu = 1;
./CxAODReader_VHbb/Root/AnalysisReader_VHbb1Lep.cxx:    passLepQuality = Props::isWHSignalMuon.get(mu) == 1;
./CxAODReader_VHbb/Root/AnalysisReader_VHcc1Lep.cxx:    if (Props::isWHSignalElectron.get(el) == 1) nSigEl = 1;
./CxAODReader_VHbb/Root/AnalysisReader_VHcc1Lep.cxx:    if (Props::isWHSignalMuon.get(mu) == 1) nSigMu = 1;
./CxAODReader_VHbb/Root/AnalysisReader_VHcc1Lep.cxx:    passLepQuality = Props::isWHSignalMuon.get(mu) == 1;
./CxAODReader_VHbb/Root/AnalysisReader_VHreso1Lep.cxx:    if (Props::isWHSignalElectron.get(m_vars.el) == 1) m_vars.nSigEl = 1;
./CxAODReader_VHbb/Root/AnalysisReader_VHreso1Lep.cxx:    if (Props::isWHSignalMuon.get(m_vars.mu) == 1) m_vars.nSigMu = 1;
./CxAODReader_VHbb/Root/AnalysisReader_VHreso1Lep.cxx:    passLepQuality = Props::isWHSignalMuon.get(m_vars.mu) == 1;
./CxAODTools/Root/CommonProperties.cxx:PROPERTY_INST( Props , int , isWHSignalElectron )
./CxAODTools/Root/CommonProperties.cxx:PROPERTY_INST( Props , int , isWHSignalMuon )
./CxAODTools/Root/OverlapRemoval.cxx:
Props::ORInputLabel.set(elec, Props::isWHSignalElectron.get(elec));// && elec->auxdata<char>("Tight"));
./CxAODTools/Root/OverlapRemoval.cxx:
Props::ORInputLabel.set(muon, Props::isWHSignalMuon.get(muon));
```

We see they are defined in `Maker_VHbb`:
```
./CxAODMaker_VHbb/Root/ElectronHandler_VHbb.cxx:  Props::isWHSignalElectron.set(electron, passSel);
./CxAODMaker_VHbb/Root/ElectronHandler_VHbb.cxx:    Props::isWHSignalElectron.copy(inElectron, outElectron);
./CxAODMaker_VHbb/Root/MuonHandler_VHbb.cxx:  Props::isWHSignalMuon.set(muon, passSel);
./CxAODMaker_VHbb/Root/MuonHandler_VHbb.cxx:    Props::isWHSignalMuon.copy(inMuon, outMuon);
```

For example for muons. We look inside `MuonHandler_VHbb.cxx`. We see `isWHSignal` muon must pass:
* `isVHSignalMuon`
* `passedIDCuts`
* `muonQuality.get(muon) < 2` (so tight or medium)
* `passWHSignalMuonIso`

What these selections mean are described in the same file.

The variable `isVHSignalMuon` is defined in `bool MuonHandler_VHbb::passVHSignalMuon(xAOD::Muon* muon){` and must pass:
* `isVHLooseMuon`
* `fabs(muon->eta()) < 2.5`
* `muon->pt() > 25000`

The variable `isVHLooseMuon` is also defined in another function `bool MuonHandler_VHbb::passVHLooseMuon(xAOD::Muon * muon)` as:
* `Props::acceptedMuonTool.get(muon)`
* `fabs(Props::d0sigBL.get(muon)) < 3`
* `fabs(Props::z0sinTheta.get(muon)) < 0.5`
* `muon->pt() > 7000`
* a cut that depends on `MJ`. If `MJ` use `Props::ptvarcone30.get(muon)/muon->pt() < 0.15`, and if if not `MJ`  use `Props::isLooseTrackOnlyIso.get(muon)`