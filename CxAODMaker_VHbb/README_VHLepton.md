For CxAODReader users, `isVHSignal` or `isWHSignal` are generally used for the lepton selection, but the definition is not clear in the reader level. This README is created to clarify what selections are applied. (Multijet sample production is not done in rel21, so the related cuts are not mentioned here) 


Electron:

`isVHLoose`
* `isLooseLH`
* `|eta|<2.47`
* `pt>7GeV`
* `|d0sigBL|<5`
* `|z0sinTheta|<0.5`
* `isGoodQO`
* `isLooseTrackOnlyIso`

`isVHSignal`(`isZHSignal`)
* `pt>25GeV`

`isWHSignal`
* `isTightLH`

Muon:

`isVHLoose`
* `combined muon`
* `|d0sigBL|<3`
* `|z0sinTheta|<0.5`
* `pt>7GeV`
* `isLooseTrackOnlyIso`

`isVHSignal`(`isZHSignal`)
* `|eta|<2.5`
* `pt>25GeV`

`isWHSignal`
* `IDCuts` (no detail in CxAODMaker_VHbb)
* `muonQuality<2` (tight or medium)
* `isFixedCutHighPtTrackOnlyIso`
