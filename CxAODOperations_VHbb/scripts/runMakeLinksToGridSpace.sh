#!/usr/bin/env bash
#
# Make links to the grid space for all the channels
#  -first make text filess of the datasets
#  -then create the symbolic links using the text files as input
#  
# Set up rucio first for making list of grid datasets
#
script=../source/CxAODOperations_VHbb/scripts/makeLinksToGridSpace.py

# tests
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_a.HIGG5D1_test.txt
outdir=/tmp/${USER}
# 
#python $script -v $infile $outdir -w 'sample_map.txt' #>& test.out

# make the maps
outdir=/tmp/${USER}
options="" #-vdt
# HIGG5D1
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_a.HIGG5D1fix.txt
python $script $infile $outdir $options -w 'sample_map_HIGG5D1_a.txt' >& writemap_HIGG5D1_a.out &
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_d.HIGG5D1.txt
python $script $infile $outdir $options -w 'sample_map_HIGG5D1_d.txt' >& writemap_HIGG5D1_d.out &
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_e.HIGG5D1.txt
python $script $infile $outdir $options -w 'sample_map_HIGG5D1_e.txt' >& writemap_HIGG5D1_e.out &

# HIGG5D2
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_a.HIGG5D2.txt
python $script $infile $outdir $options -w 'sample_map_HIGG5D2_a.txt' >& writemap_HIGG5D2_a.out &
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_d.HIGG5D2.txt
python $script $infile $outdir $options -w 'sample_map_HIGG5D2_d.txt' >& writemap_HIGG5D2_d.out &
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_e.HIGG5D2.txt
python $script $infile $outdir $options -w 'sample_map_HIGG5D2_e.txt' >& writemap_HIGG5D2_e.out &

# HIGG2D4
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_a.HIGG2D4.txt
python $script $infile $outdir $options -w 'sample_map_HIGG2D4_a.txt' >& writemap_HIGG2D4_a.out &
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_d.HIGG2D4.txt
python $script $infile $outdir $options -w 'sample_map_HIGG2D4_d.txt' >& writemap_HIGG2D4_d.out &
infile=../source/CxAODOperations_VHbb/data/CxAOD/CxAOD_r32-15/out_sample_list_sample_grid.13TeV_25ns.mcdata_e.HIGG2D4.txt
python $script $infile $outdir $options -w 'sample_map_HIGG2D4_e.txt' >& writemap_HIGG2D4_e.out &

# make the links
# HIGG5D1
#outdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15_grid/HIGG5D1_13TeV/CxAOD_32-15_a/
#python $script -v dummyPath $outdir -r sample_map_HIGG5D1_a.txt >& link_HIGG5D1_a.out &
#outdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15_grid/HIGG5D1_13TeV/CxAOD_32-15_d/
#python $script -v dummyPath $outdir -r sample_map_HIGG5D1_d.txt >& link_HIGG5D1_d.out &
#outdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15_grid/HIGG5D1_13TeV/CxAOD_32-15_e/
#python $script -v dummyPath $outdir -r sample_map_HIGG5D1_e.txt >& link_HIGG5D1_e.out &

# HIGG5D2
#outdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15_grid/HIGG5D2_13TeV/CxAOD_32-15_a/
#python $script -v dummyPath $outdir -r sample_map_HIGG5D2_a.txt >& link_HIGG5D2_a.out &
#outdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15_grid/HIGG5D2_13TeV/CxAOD_32-15_d/
#python $script -v dummyPath $outdir -r sample_map_HIGG5D2_d.txt >& link_HIGG5D2_d.out &
#outdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15_grid/HIGG5D2_13TeV/CxAOD_32-15_e/
#python $script -v dummyPath $outdir -r sample_map_HIGG5D2_e.txt >& link_HIGG5D2_e.out &

# HIGG2D4
#outdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15_grid/HIGG2D4_13TeV/CxAOD_32-15_a/
#python $script -v dummyPath $outdir -r sample_map_HIGG2D4_a.txt >& link_HIGG2D4_a.out &
#outdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15_grid/HIGG2D4_13TeV/CxAOD_32-15_d/
#python $script -v dummyPath $outdir -r sample_map_HIGG2D4_d.txt >& link_HIGG2D4_d.out &
#outdir=/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15_grid/HIGG2D4_13TeV/CxAOD_32-15_e/
#python $script -v dummyPath $outdir -r sample_map_HIGG2D4_e.txt >& link_HIGG2D4_e.out &

