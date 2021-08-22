#!/bin/sh
RELEASE=`cat CxAODBootstrap_VHbb/bootstrap/release.txt`
cd ../build
setupATLAS
lsetup git
acmSetup --sourcedir=../source AnalysisBase,${RELEASE},here
acm sparse_clone_project athena
acm add_pkg athena/Reconstruction/Jet/JetCalibTools
cd ../source/athena/Reconstruction/Jet/JetCalibTools
cp -r /afs/cern.ch/user/t/tnobe/public/TCCconfigs share
cd -
acm find_packages
acm compile
