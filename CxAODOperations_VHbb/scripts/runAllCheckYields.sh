#!/bin/bash
#
# Run for all years and derivations the comparison of 
# the latest yields from AMI with the DxAOD yields from the production
#
script=../source/CxAODOperations_VHbb/scripts/checkYields.py
pathtoamifiles=/afs/cern.ch/user/v/vhbbframework/public/NevtDxAOD_32-15/
#../source/CxAODOperations_VHbb/data/DxAOD/VHbb
pathtoyieldsfiles=/home/$USER
outdirectory=./ 
python $script -a ${pathtoamifiles}/dslist_NevtDxAOD_HIGG5D1_a.txt \
    -y ${pathtoyieldsfiles}/yields.13TeV_DxAOD_sorted_32-15_HIGG5D1_a.txt \
    -o ${outdirectory}/dslist_NevtDxAOD_CxAODfraction_32-15_HIGG5D1_a.txt
python $script -a ${pathtoamifiles}/dslist_NevtDxAOD_HIGG5D1_d.txt \
    -y ${pathtoyieldsfiles}/yields.13TeV_DxAOD_sorted_32-15_HIGG5D1_d.txt \
    -o ${outdirectory}/dslist_NevtDxAOD_CxAODfraction_32-15_HIGG5D1_d.txt
python $script -a ${pathtoamifiles}/dslist_NevtDxAOD_HIGG5D1_e.txt \
    -y ${pathtoyieldsfiles}/yields.13TeV_DxAOD_sorted_32-15_HIGG5D1_e.txt \
    -o ${outdirectory}/dslist_NevtDxAOD_CxAODfraction_32-15_HIGG5D1_e.txt
#
python $script -a ${pathtoamifiles}/dslist_NevtDxAOD_HIGG5D2_a.txt \
    -y ${pathtoyieldsfiles}/yields.13TeV_DxAOD_sorted_32-15_HIGG5D2_a.txt \
    -o ${outdirectory}/dslist_NevtDxAOD_CxAODfraction_32-15_HIGG5D2_a.txt
python $script -a ${pathtoamifiles}/dslist_NevtDxAOD_HIGG5D2_d.txt \
    -y ${pathtoyieldsfiles}/yields.13TeV_DxAOD_sorted_32-15_HIGG5D2_d.txt \
    -o ${outdirectory}/dslist_NevtDxAOD_CxAODfraction_32-15_HIGG5D2_d.txt
python $script -a ${pathtoamifiles}/dslist_NevtDxAOD_HIGG5D2_e.txt \
    -y ${pathtoyieldsfiles}/yields.13TeV_DxAOD_sorted_32-15_HIGG5D2_e.txt \
    -o ${outdirectory}/dslist_NevtDxAOD_CxAODfraction_32-15_HIGG5D2_e.txt
#
python $script -a ${pathtoamifiles}/dslist_NevtDxAOD_HIGG2D4_a.txt \
    -y ${pathtoyieldsfiles}/yields.13TeV_DxAOD_sorted_32-15_HIGG2D4_a.txt \
    -o ${outdirectory}/dslist_NevtDxAOD_CxAODfraction_32-15_HIGG2D4_a.txt
python $script -a ${pathtoamifiles}/dslist_NevtDxAOD_HIGG2D4_d.txt \
    -y ${pathtoyieldsfiles}/yields.13TeV_DxAOD_sorted_32-15_HIGG2D4_d.txt \
    -o ${outdirectory}/dslist_NevtDxAOD_CxAODfraction_32-15_HIGG2D4_d.txt
python $script -a ${pathtoamifiles}/dslist_NevtDxAOD_HIGG2D4_e.txt \
    -y ${pathtoyieldsfiles}/yields.13TeV_DxAOD_sorted_32-15_HIGG2D4_e.txt \
    -o ${outdirectory}/dslist_NevtDxAOD_CxAODfraction_32-15_HIGG2D4_e.txt