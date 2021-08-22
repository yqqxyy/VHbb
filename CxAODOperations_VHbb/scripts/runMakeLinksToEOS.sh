#!/usr/bin/env bash
#
# Make links to eos for all the channels
#  -first make text files of the full path to the existing eos datasets (using find)
#  -then create the symbolic links from a specified directory using the text files as input
#  
# assume working from a directory parallel to CxAODOperations
script=../source/CxAODOperations_VHbb/scripts/makeLinksToEOS.py
sample_info=../source/CxAODOperations_VHbb/data/DxAOD/info/sample_info.txt

# 

makefilelists=1
makeeoslinks=0

if [ $makefilelists -eq 1 ] 
then
    # make the file lists
    outdir=./  
    # HIGG5D1
    find /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/CxAOD_r32-15/HIGG5D1_13TeV/CxAOD_32-15_a/*/*_CxAOD.root/*root* > $outdir/list_HIGG5D1_a.txt
    find /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/CxAOD_r32-15/HIGG5D1_13TeV/CxAOD_32-15_d/*/*_CxAOD.root/*root* > $outdir/list_HIGG5D1_d.txt
    find /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/CxAOD_r32-15/HIGG5D1_13TeV/CxAOD_32-15_e/*/*_CxAOD.root/*root* > $outdir/list_HIGG5D1_e.txt

    # HIGG5D2
    # being copied to eos

    # HIGG2D4
    find /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/CxAOD_r32-15/HIGG2D4_13TeV/CxAOD_32-15_a/*/*_CxAOD.root/*root* > $outdir/list_HIGG2D4_a.txt
    find /eos/atlas/unpledged/group-tokyo/users/ynoguchi/group_higgs/CxAOD/CxAOD_r32-15-01/HIGG2D4_13TeV/CxAOD_32-15-01_d/*/*_CxAOD.root/*root* > $outdir/list_HIGG2D4_d.txt
    find /eos/atlas/unpledged/group-tokyo/users/ynoguchi/group_higgs/CxAOD/CxAOD_r32-15-01/HIGG2D4_13TeV/CxAOD_32-15-01_e/*/*_CxAOD.root/*root* > $outdir/list_HIGG2D4_e.txt
fi

# make the links
if [ $makeeoslinks -eq 1 ] 
then
    options=-v #"" #-vdt verbose, debug, testonly
    stemdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15-with-links

    # HIGG5D1
    outdir=$stemdir/HIGG5D1_13TeV/CxAOD_32-15_a/
    python $script $options -s $sample_info list_HIGG5D1_a.txt $outdir  >& link_HIGG5D1_a.out &
    outdir=$stemdir/HIGG5D1_13TeV/CxAOD_32-15_d/
    python $script $options -s $sample_info list_HIGG5D1_d.txt $outdir  >& link_HIGG5D1_d.out &
    outdir=$stemdir/HIGG5D1_13TeV/CxAOD_32-15_e/
    python $script $options -s $sample_info list_HIGG5D1_e.txt $outdir  >& link_HIGG5D1_e.out &
    
    # HIGG5D2

    
    # HIGG2D4
    outdir=$stemdir/HIGG2D4_13TeV/CxAOD_32-15_a/
    python $script $options -s $sample_info list_HIGG2D4_a.txt $outdir  >& link_HIGG2D4_a.out &
    outdir=$stemdir/HIGG2D4_13TeV/CxAOD_32-15_d/
    python $script $options -s $sample_info list_HIGG2D4_d.txt $outdir  >& link_HIGG2D4_d.out &
    outdir=$stemdir/HIGG2D4_13TeV/CxAOD_32-15_e/
    python $script $options -s $sample_info list_HIGG2D4_e.txt $outdir  >& link_HIGG2D4_e.out &
fi
