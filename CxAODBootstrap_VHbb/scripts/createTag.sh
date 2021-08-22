#!/bin/bash

if [ $# -ne 2 ]; then
cat <<EOF
Usage: $0 TAG    TEXT
Usage: $0 r29-00 "First test production in R21, on Nov 22"
Usage: $0 r30-00 "15Jan2018"
Usage: $0 r30-01 "15Jan2018"
Usage: $0 r30-02 "16Jan2018 # 21.2.14 prod"
Usage: $0 r30-03 "23Jan2018 # 21.2.15 trigger match fixed"
Usage: $0 r30-04 "08Feb2018 # 21.2.17 var-R track jet added"
Usage: $0 r30-05 "08Feb2018-2 # master->dev-R20; more MR in dev-R21"
Usage: $0 r30-06 "10Feb2018 # dev-R21->master # did actually from another branch, redo"
Usage: $0 r30-07 "10Feb2018-2 # tag of master after dev-R21->master"
Usage: $0 r30-08 "10Feb2018-3 # tag of dev-R21 after dev-R21->master"
Usage: $0 r30-09 "12Feb2018-R21 # tag of dev-R21 after dev-R21->master and pTag bug fix"
Usage: $0 r30-10 "12Feb2018-MAS # tag of master after dev-R21->master and pTag bug fix"
Usage: $0 r30-11 "22Feb2018 21.2.19 fat and jet uncertainties fixed"
Usage: $0 r30-12 "02Mar2018 21.2.20 CP tools updated"
Usage: $0 r30-13 "05Mar2018 21.2.20 PRW tool 1.09 -> 1.03"
Usage: $0 r30-14 "05Mar2018 21.2.20 PRW tool 1.09 -> 1.03 Only now added"
Usage: $0 r30-15 "07Mar2018 21.2.20 electron isolation bug fix"
Usage: $0 r30-16 "09Mar2018 21.2.22"
Usage: $0 r30-17 "16Mar2018 21.2.23"
Usage: $0 r30-18 "25Mar2018 21.2.23 after PRW restructure"
Usage: $0 r31-01 "26Mar2018 21.2.23 after PRW restructure; same as 30-18"
Usage: $0 r31-02 "04Apr2018 21.2.23 some PRW fixes, same physics as 31-01"
Usage: $0 r31-03 "04Apr2018 21.2.23 bug in previous tag, this one really the latest master, same physics as 31-01"
Usage: $0 r31-04 "23Apr2018 21.2.23 no physics changes; updates on scripts; fixes in Reader"
Usage: $0 r31-05 "24Apr2018 21.2.23 allows to recompute PRW at Reader, for this also stores two extra variables in CxAOD"
Usage: $0 r31-06 "27Apr2018 21.2.27 and PRW updated to actual mu; updated sample lists"
Usage: $0 r31-07 "27Apr2018 21.2.27 and PRW updated to actual mu; updated sample lists; previous tag was not from the latest code"
Usage: $0 r31-08 "28Apr2018 21.2.27 updated scripts and sample lists (new naming convention, only mcdata and test, actual mu as default"
Usage: $0 r31-09 "29Apr2018 21.2.27 fixed a typo/bug in scripts and added README on CxAOD production and download"
Usage: $0 r31-10 "30Apr2018 21.2.27 tag of v31-10 used for CxAOD with systematics"
Usage: $0 r31-11 "22May2018 21.2.30 more truth info for AZH more advances on Readers changes in Makers do not affect results"
Usage: $0 r31-12 "24Jun2018 21.2.34 Maker with syst reduced CPU by 70%, 0L inputs for ICHEP"
Usage: $0 r31-13 "24Jun2018 21.2.34 Add MVA from Oxford in 0L; moved (gg)WW from direct to truth tagging"
Usage: $0 r31-14 "25Jun2018 21.2.34 Reduce CPU time for 0L; store MVA-inputs if doReduced"
Usage: $0 r31-15 "15Jul2018 21.2.37 Updated samples mcdata and mcdataVHbb a,d"
Usage: $0 r31-16 "01Aug2018 21.2.39 Master is once again reproducing the ICHEP results in 0L and 1L"
Usage: $0 r31-17 "06Aug2018 21.2.39 CorrsAndSysts now depends on CxAODReader_VHbb, not on CxAODReader"
Usage: $0 r31-18 "07Aug2018 21.2.39 First tag without FrameworkExe & FramworkSub, but with CxAODBootstrap_VHbb, CxAODOperations and CxAODOperations_VHbb"
Usage: $0 r31-19 "25Aug2018 21.2.40 Tau-el OR added in Maker, submitReader.sh reproduces ICHEP results for 0L and 1L"
Usage: $0 r31-20 "25Aug2018 21.2.40 only now 1L Reader reproduces ICHEP, TCC jet added as option in Maker, CorrsAndSysts bug added for 2L"
Usage: $0 r31-21 "14Sep2018 21.2.43 Various changes; before starting to merge the 2L VH(bb) changes"
Usage: $0 r31-22 "16Sep2018 21.2.43 After merging 2L VH(bb) changes, validated that all of 0L/1L/2L in this tag reproduce ICHEP"
Usage: $0 r31-23 "28Sep2018 21.2.45 After reducing CxAOD size by 30% by removing many MET significance quantities"
Usage: $0 r31-24 "25Oct2018 21.2.49 After several CP updates, adding CoM jets, pipelines in new derivations 3 signal samples"
Usage: $0 r31-25 "03Nov2018 21.2.51 After several CP updates (el/mu/photon), fixed Jet JER, fat jets (regular & TCC), redefine MET Truth"
Usage: $0 r32-00 "05Nov2018 21.2.51 After b-jet energy corrections stored in new style of R21 with VR muon or else; Reader backward compatible with ICHEP VHbb, reduce other sizes too" 
Usage: $0 r32-01 "12Nov2018 21.2.52 After object size reduction is thought to be done; used to validate that"
Usage: $0 r32-02 "14Nov2018 21.2.52 Update 2017 GRL; updated Muon fixed cut tight iso WP; fixed saggita corr for 2017" 
Usage: $0 r32-03 "20Nov2018 21.2.52 Bug fixes so that mc16e can run; updated sample lists; same physics as r32-02"
Usage: $0 r32-04 "07Dec2018 21.2.56 CP updates Jet, MET, muon, etc"
Usage: $0 r32-05 "08Dec2018 21.2.56 A few more CP; Updated operations of Maker with test2 for VHbb with METTrigger skimming fix"
Usage: $0 r32-06 "11Dec2018 21.2.57 A few more CP; truth info"
Usage: $0 r32-07 "04Jan2019 21.2.58 Added more truth info for filtered samples and truth matched to reco; removed DBL signals"
Usage: $0 r32-08 "24Jan2019 21.2.60 Added lepton isolation WP of look track only, but set to false as default" 
Usage: $0 r32-09 "12Feb2019 21.2.60 Maker truth working also for older truth style derivations; Reader 1L filtering ttbar/STopWt fixed" 
Usage: $0 r32-10 "15Feb2019 21.2.60 After MVA harmonisation in CxAODReader_VHbb; added CoM info in Reader"
Usage: $0 r32-11 "21Feb2019 21.2.60 After all the extensions validated in submitReader.sh, MET trigger SF added"
Usage: $0 r32-12 "25Feb2019 21.2.64; new CP tools update; fix PFlow in Maker; fix not apply MET rebuilding for truth MET, add CoM in selections"
Usage: $0 r32-13 "26Feb2019 21.2.64; update PtReco trained in 32-07 ade, replacing the previous trained in 31-10 ad, in both Maker and Reader"
Usage: $0 r32-14 "26Mar2019 21.2.66; update electron isolation variables written, cuts on NJet, pTV for preselection and fatjet systematics"
Usage: $0 r32-15 "01Apr2019 21.2.68: update Analysis Base for muon fixes, fatjet protection for data, small fatjet muon fix"
Usage: $0 r32-16 "01Apr2019 21.2.72: update Analysis Base for PFlow recommendations, remove events with electrons in crack for MET in mc15a/2015/16 data"
Usage: $0 r32-17 "02Nov2019 21.2.90: tag for WWW analysis"
Usage: $0 r32-18 "08Nov2019 21.2.90: tag after VBS updates"
Usage: $0 r32-19 "08Nov2019 21.2.96: tag once with new AB for VBS production"
Usage: $0 r32-20 "09Nov2019 21.2.89: tag for VBS production before egamma bug"
Usage: $0 r32-21 "10Nov2019 21.2.89: tag after adding a guard for mc-only variables in FatjetHandler"
Usage: $0 r32-22 "for test; same as 32-21"
Usage: $0 r32-23 "18Nov2019 21.2.89: tag for WWW analysis after bug fix"
Usage: $0 r32-24-Reader-01 "Reader tag for resolved VHbb post-M2 input production"    #only for CxAODBootstrap_VHbb, same as v2 and v3
Usage: $0 r32-24-Reader-02 "Reader tag for resolved VHbb post-M2 input production v2" #only for CxAODBootstrap_VHbb, same as v1 and v3
Usage: $0 r32-24-Reader-03 "Reader tag for resolved VHbb post-M2 input production v3"
Usage: $0 r32-24-Reader-04 "Reader tag for resolved VHbb post-M2 input production v4; v3 including two fixes" 
Usage: $0 r32-24-Reader-boosted-01 "Reader tag for first full boosted VHbb input production"
Usage: $0 r32-24-Reader-boosted-02 "FINAL Reader tag for first full boosted VHbb input production"
EOF
exit 1
fi 

TAG=$1
TEXT=$2

for d in *; do
    if [ -d "${d}" ]; then
        echo "${d}"   # your processing here
	#if [ "${d}" == "FrameworkSub" ]; then
	#    continue
	#fi
	#continue
	cd $d
	git branch -a
	git log | head
	git tag -a ${TAG} -m "${TEXT}"
	git tag -l | head
	git show ${TAG} | head
	git push origin ${TAG}	
	cd ..
    fi 
    #git submodule add ./$d
done
