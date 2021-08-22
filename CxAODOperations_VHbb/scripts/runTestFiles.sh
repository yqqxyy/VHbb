#
# Script for running test files on afs
# 
# Script runs one job at a time interactively

#path to Maker script
script=../source/CxAODOperations_VHbb/scripts/submitMaker.sh 

# choose channels, number of events and samples
do0L=1
do1L=1
do2L=1
ndata=1000
nmc=1000
doData=0
doSig=1
doTop=0

# Derivations on afs
# data and signals
data16_0lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data16_13TeV.00311321.physics_Main.deriv.DAOD_HIGG5D1.r9264_p3083_p3640
data17_0lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data17_13TeV.00327761.physics_Main.deriv.DAOD_HIGG5D1.r10203_p3399_p3640 
data18_0lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data18_13TeV.00364292.physics_Main.deriv.DAOD_HIGG5D1.f1002_m2037_p3718

sigmc16a_0lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.345056.PowhegPythia8EvtGen_NNPDF3_AZNLO_ZH125J_MINLO_vvbb_VpT.deriv.DAOD_HIGG5D1.e5706_e5984_s3126_r9364_r9315_p3641
sigmc16d_0lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.345056.PowhegPythia8EvtGen_NNPDF3_AZNLO_ZH125J_MINLO_vvbb_VpT.deriv.DAOD_HIGG5D1.e5706_e5984_s3126_r10201_r10210_p3641 
sigmc16e_0lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.345056.PowhegPythia8EvtGen_NNPDF3_AZNLO_ZH125J_MINLO_vvbb_VpT.deriv.DAOD_HIGG5D1.e5706_s3126_r10724_p3717

data16_1lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data16_13TeV.00311321.physics_Main.deriv.DAOD_HIGG5D2.r9264_p3083_p3640 
data17_1lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data17_13TeV.00327761.physics_Main.deriv.DAOD_HIGG5D2.r10203_p3399_p3640 
data18_1lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data18_13TeV.00364292.physics_Main.deriv.DAOD_HIGG5D2.f1002_m2037_p3718 

sigmc16a_1lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.345053.PowhegPythia8EvtGen_NNPDF3_AZNLO_WmH125J_MINLO_lvbb_VpT.deriv.DAOD_HIGG5D2.e5706_e5984_s3126_r9364_r9315_p3641 
sigmc16d_1lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.345053.PowhegPythia8EvtGen_NNPDF3_AZNLO_WmH125J_MINLO_lvbb_VpT.deriv.DAOD_HIGG5D2.e5706_e5984_s3126_r10201_r10210_p3641 
sigmc16e_1lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.345053.PowhegPythia8EvtGen_NNPDF3_AZNLO_WmH125J_MINLO_lvbb_VpT.deriv.DAOD_HIGG5D2.e5706_s3126_r10724_p3717

data16_2lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data16_13TeV.00311321.physics_Main.deriv.DAOD_HIGG2D4.r9264_p3083_p3640 
data17_2lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data17_13TeV.00327761.physics_Main.deriv.DAOD_HIGG2D4.r10203_p3399_p3640 
data18_2lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data18_13TeV.00364292.physics_Main.deriv.DAOD_HIGG2D4.f1002_m2037_p3640 

sigmc16a_2lep=/afs//cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.345055.PowhegPythia8EvtGen_NNPDF3_AZNLO_ZH125J_MINLO_llbb_VpT.deriv.DAOD_HIGG2D4.e5706_e5984_s3126_r9364_r9315_p3641 
sigmc16d_2lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.345055.PowhegPythia8EvtGen_NNPDF3_AZNLO_ZH125J_MINLO_llbb_VpT.deriv.DAOD_HIGG2D4.e5706_e5984_s3126_r10201_r10210_p3641 
sigmc16e_2lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.345055.PowhegPythia8EvtGen_NNPDF3_AZNLO_ZH125J_MINLO_llbb_VpT.deriv.DAOD_HIGG2D4.e5706_e5984_s3126_r10724_r10726_p3641




# ttbar
ttbarmc16a_0lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_HIGG5D1.e6337_e5984_s3126_r9364_r9315_p3639
ttbarmc16d_0lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_HIGG5D1.e6337_e5984_s3126_r10201_r10210_p3639
ttbarmc16e_0lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_HIGG5D1.e6337_s3126_r10724_p3717

ttbarmc16a_1lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_HIGG5D2.e6337_e5984_s3126_r9364_r9315_p3639
ttbarmc16d_1lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_HIGG5D2.e6337_e5984_s3126_r10201_r10210_p3639
ttbarmc16e_1lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_HIGG5D2.e6337_s3126_r10724_p3717

ttbarmc16a_2lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_HIGG2D4.e6348_e5984_s3126_r9364_r9315_p3639
ttbarmc16d_2lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_HIGG2D4.e6348_e5984_s3126_r10201_r10210_p3639
ttbarmc16e_2lep=/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_HIGG2D4.e6348_e5984_s3126_r10724_r10726_p3639

# Reminder of the inputs to submitMaker:
#CHANNELs=$1
#MCTYPEs=$2
#DERIV=$3
#STEM=$4
#VTAG=$5
#GRID_PRIVILEGE=$6
#USE_PF=$7
#USE_TCC=$8 <-- DBL
#NR_EVENTS=$9
#LOCAL_SAMPLE=${10}
#LOG_FILE=${11}
#DO_EXECUTE=${12}
PF=1
TCC=0

if [ "$doData" = '1' ] ; then
    if [ "$do0L" = '1' ] ; then
	source $script 0L a VHbb none none Higgs $PF $TCC $ndata $data16_0lep none 1 >& run_0L_data16_sys.out
	source $script 0L d VHbb none none Higgs $PF $TCC $ndata $data17_0lep none 1 >& run_0L_data17_sys.out
	source $script 0L e VHbb none none Higgs $PF $TCC $ndata $data18_0lep none 1 >& run_0L_data18_sys.out
    fi
    if [ "$do1L" = '1' ] ; then
	source $script 1L a VHbb none none Higgs $PF $TCC $ndata $data16_1lep none 1 >& run_1L_data16_sys.out
	source $script 1L d VHbb none none Higgs $PF $TCC $ndata $data17_1lep none 1 >& run_1L_data17_sys.out
	source $script 1L e VHbb none none Higgs $PF $TCC $ndata $data18_1lep none 1 >& run_1L_data18_sys.out
    fi
    if [ "$do2L" = '1' ] ; then
	source $script 2L a VHbb none none Higgs $PF $TCC $ndata $data16_2lep none 1 >& run_2L_data16_sys.out
	source $script 2L d VHbb none none Higgs $PF $TCC $ndata $data17_2lep none 1 >& run_2L_data17_sys.out
	source $script 2L e VHbb none none Higgs $PF $TCC $ndata $data18_2lep none 1 >& run_2L_data18_sys.out
    fi
fi

if [ "$doSig" = '1' ] ; then
    if [ "$do0L" = '1' ] ; then
	source $script 0L a VHbb none none Higgs $PF $TCC $nmc  $sigmc16a_0lep none 1 >& run_0L_mc16a_sig_sys.out
	source $script 0L d VHbb none none Higgs $PF $TCC $nmc  $sigmc16d_0lep none 1 >& run_0L_mc16d_sig_sys.out
	source $script 0L e VHbb none none Higgs $PF $TCC $nmc  $sigmc16e_0lep none 1 >& run_0L_mc16e_sig_sys.out
    fi
    if [ "$do1L" = '1' ] ; then
	source $script 1L a VHbb none none Higgs $PF $TCC $nmc  $sigmc16a_1lep none 1 >& run_1L_mc16a_sig_sys.out
	source $script 1L d VHbb none none Higgs $PF $TCC $nmc  $sigmc16d_1lep none 1 >& run_1L_mc16d_sig_sys.out
	source $script 1L e VHbb none none Higgs $PF $TCC $nmc  $sigmc16e_1lep none 1 >& run_1L_mc16e_sig_sys.out
    fi
    if [ "$do2L" = '1' ] ; then
	source $script 2L a VHbb none none Higgs $PF $TCC $nmc  $sigmc16a_2lep none 1 >& run_2L_mc16a_sig_sys.out
	source $script 2L d VHbb none none Higgs $PF $TCC $nmc  $sigmc16d_2lep none 1 >& run_2L_mc16d_sig_sys.out
	source $script 2L e VHbb none none Higgs $PF $TCC $nmc  $sigmc16e_2lep none 1 >& run_2L_mc16e_sig_sys.out    
    fi
fi
    
if [ "$doTop" = '1' ] ; then
    if [ "$do0L" = '1' ] ; then
	source $script 0L a VHbb none none Higgs $PF $TCC $nmc  $ttbarmc16a_0lep none 1 >& run_0L_mc16a_ttbar_sys.out
	source $script 0L d VHbb none none Higgs $PF $TCC $nmc  $ttbarmc16d_0lep none 1 >& run_0L_mc16d_ttbar_sys.out
	source $script 0L e VHbb none none Higgs $PF $TCC $nmc  $ttbarmc16e_0lep none 1 >& run_0L_mc16e_ttbar_sys.out
    fi
    if [ "$do1L" = '1' ] ; then
	source $script 1L a VHbb none none Higgs $PF $TCC $nmc  $ttbarmc16a_1lep none 1 >& run_1L_mc16a_ttbar_sys.out
	source $script 1L d VHbb none none Higgs $PF $TCC $nmc  $ttbarmc16d_1lep none 1 >& run_1L_mc16d_ttbar_sys.out
	source $script 1L e VHbb none none Higgs $PF $TCC $nmc  $ttbarmc16e_1lep none 1 >& run_1L_mc16e_ttbar_sys.out
    fi
    if [ "$do2L" = '1' ] ; then
	source $script 2L a VHbb none none Higgs $PF $TCC $nmc  $ttbarmc16a_2lep none 1 >& run_2L_mc16a_ttbar_sys.out
	source $script 2L d VHbb none none Higgs $PF $TCC $nmc  $ttbarmc16d_2lep none 1 >& run_2L_mc16d_ttbar_sys.out
	source $script 2L e VHbb none none Higgs $PF $TCC $nmc  $ttbarmc16e_2lep none 1 >& run_2L_mc16e_ttbar_sys.out    
    fi
fi
