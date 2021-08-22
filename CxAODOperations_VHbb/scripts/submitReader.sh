#!/usr/bin/env bash
# if there is no parameter, it stops and it gives the instructions
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group  

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 != "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script forcing you to execute. Don't source me!" >&2
    return 1
else
    # if I am OK to execute, force that the script stops if variables are not defined
    # this catches bugs in the code when you think a variable has value, but it is empty
    set -eu
fi

# if there is no parameter, it stops and it gives the instructions
if [ $# -ne 11 ]; then
cat <<EOF
Usage: $0 InputFolder                              OutputFolder                                       Channel  MCType Analysis Model   Tag      VTags        SAMPLESFINAL1         SAMPLESFINAL2  DoExecute
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/180609_01              0L       a,d    VHbb     MVA,CUT D,T,H    31-10-ICHEP  none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/180609_01              0L       a,d    VHbb     MVA,CUT D1,D2,T2 31-10-ICHEP  none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/180614_01              0L       a,d    VHbb     MVA,CUT D1,T2    31-10-ICHEP  none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/180614_01              0L       a,d    VHbb     MVA,CUT D1,T2    31-10-ICHEP  none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/180804_01              0L       a      VHbb     MVA     D1       31-10-ICHEP  none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/180925_01              2L       a      VHcc     CUT     D1       31-15        none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/181004_TrainTree_0L_01 0L       a,d    VHbb     MVA     T        31-10-ICHEP  none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/181108_Test            0L,1L,2L a,d,e  VHbb     MVA     D1       31-25        none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/181108_Test_data18     0L       e      VHbb     MVA     D1       31-25        none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/181119_Test_01         0L       e      VHbb     MVA     D1       32-02        ttbar_nonallhad_PwPy8 ggWqqWlv_Sh222 1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/181226_TestEM_01       0L       e      VHbb     MVA     D1       32-02-EM     ttbar_nonallhad_PwPy8 ggWqqWlv_Sh222 1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/181226_TestPF_01       0L       e      VHbb     MVA     D1       32-02-PF     ttbar_nonallhad_PwPy8 ggWqqWlv_Sh222 1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/190128_Test_01         0L       e      VHbb     MVA     D1       32-02-7      qqZvvHbbJ_PwPy8MINLO  ggWqqWlv_Sh222 1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/190128_Test_01         0L,1L,2L a,d,e  VHbb     MVA     D1,T2    31-15,32-07  none                  none           1
Usage: $0 /data06/abuzatu/data/CxAOD/ToUseInReader /data06/abuzatu/data/Reader/190128_Test_01         0L,1L,2L a,d,e  VHbb     MVA     D1       31-15,32-07  none                  none           1
Usage: 
Usage: Tag: D1 is direct for the samples that use only direct (all besides V+jets C and L filter)
Usage: Tag: D2 is direct for the samples that can use either direct or truth (V+jets C and L filter)  
Usage: Tag: T2 is truth  for the samples that can use either direct or truth (V+jets C and L filter)  
Usage: Tag: D, T or H are all the samples with either direct, or truth or hybrid tagging respectively.
Usage: RunVHcc: optional argument, if set to 1 VHcc setup is used, otherwise VHbb
Usage: Typical SAMPLESFINAL1: qqZvvHbbJ_PwPy8MINLO, qqZllHbbJ_PwPy8MINLO qqWlvHbbJ_PwPy8MINLO ggZvvHbb_PwPy8 ggZllHbb_PwPy8
Usage: Typical SAMPLESFINAL2: ggWqqWlv_Sh222, WmunuL_Sh221, ZmumuL_Sh221
EOF
exit 1
fi

# #########################################################################################################
# COMMAND LINE ARGUMENTS                                                                          #########
# #########################################################################################################

INPUT_FOLDER=$1
OUTPUT_FOLDER=$2
CHANNELs=$3
MCTYPEs=$4
ANALYSIS=$5
MODELTYPEs=$6
TAGGINGs=$7
VTAGs=$8
SAMPLESFINAL1=$9
SAMPLESFINAL2=${10}
DO_EXECUTE=${11}

echo ""
echo ""
echo ""
echo "Start submitReader.sh"
echo "INPUT_FOLDER=${INPUT_FOLDER}"
echo "OUTPUT_FOLDER=${OUTPUT_FOLDER}"
echo "CHANNELs=${CHANNELs}"
echo "MCTYPEs=${MCTYPEs}"
echo "ANALYSIS=${ANALYSIS}"
echo "MODELTYPEs=${MODELTYPEs}"
echo "TAGGINGs=${TAGGINGs}"
echo "VTAGs=${VTAGs}"
echo "SAMPLESFINAL1=${SAMPLESFINAL1}"
echo "SAMPLESFINAL2=${SAMPLESFINAL2}"
echo "DO_EXECUTE=${DO_EXECUTE}"

# #########################################################################################################
# Check that the CxAOD framework environment has been set up                                    ###########
# #########################################################################################################

# check if the CxAODFramework has been set up
if [[ ${ALRB_availableTools} == "" ]]; then
    echo "Cannot find asetup, as ALRB_availableTools is empty. Did you do setupATLAS?"
    return 1
fi
#
if [[ ${ALRB_availableTools} != *"asetup"* ]]; then
    echo "Cannot find asetup, as ALRB_availableTools is not empty, but does not contain asetup. Did you do setupATLAS?"
    return 1
fi
echo "ALRB_availableTools=${ALRB_availableTools}"
#
if [[ ${AtlasVersion} == "" ]]; then
    echo "AtlasVersion is not set. Did you do asetup?"
    return 1
fi
echo "AtlasVersion=${AtlasVersion}"
#
if [[ ${WorkDir_DIR} == "" ]]; then
    echo "WorkDir_DIR is not set. Did you do: cd build && source x86_64-*-gcc*-opt/setup.sh ?"
    return 1
fi
echo "WorkDir_DIR=${WorkDir_DIR}"
# if we need to set up the CxAOD we do like this, assuming we are in the run folder
# source ../source/CxAODOperations_VHbb/scripts/setupLocal.sh

# which analysis strategy to use?
ANASTRATEGY="Merged" # Resolved, Merged, SimpleMerge500, PriorityResolvedSR, PriorityMergedSR, etc

#global setting to use Pseudo-continuous b-tagging 
USE_PCBT="0"

# Set USE_PCBT to true if VHbb resolved
if [[ ${ANALYSIS} == "VHbb" ]] && [[ ${ANASTRATEGY} == "Resolved" ]]; then
    USE_PCBT="1"
fi

# #########################################################################################################
# To produce the MVA training trees we want to set in one go all the settings
# #########################################################################################################

DO_MVA_TRAINING_TREES="0"

# #########################################################################################################
# Debug, on how many events and where to run                                                    ###########
# #########################################################################################################

DEBUG="false"
NUMBEROFEVENTS="-1"

# infos particular to batch systems 
DRIVER="condor" # condor, direct, LSF
CONDOROSPREFERENCE="CentOS7" #if you use the default of your batch system, set to "none"
BQUEUE="8nh" # for LSF driver
# Possible condor queues:
# none (default queue), espresso (CERN default, 20min walltime), microcentury (1h), longlunch (2h), workday (8h), tomorrow (1d), testmatch (3d), nextweek (1w)
# Possible eos folder name stem it should be in the user eos area, where you hae the right to write, and not the Higgs eos area
# it is a stem as we will add automatically to it the DESCRIPTION="${CHANNEL}_${VTAG}_${MCTYPE}_${MODELTYPE}_${TAGGING}"
# so that we can still submit all cases in one go, just as we do for the histograms
# if you select none, the training trees will go in the same place as the histograms
# e.g. /eos/user/m/mdacunha/CONDOR_output/testDir, but note that the base folder must exist, i.e. /eos/user/m/mdacunha/CONDOR_output already
if [[ ${DO_MVA_TRAINING_TREES} == "1" ]]; then
    CONDORQUEUE="workday"
    # EOSFOLDERUSERSTEM="/eos/user/m/mdacunha/CONDOR_output/ChooseYourFolder"
    EOSFOLDERUSERSTEM="none"
else
    # running histograms only
    CONDORQUEUE="workday" 
    EOSFOLDERUSERSTEM="none"
fi
RSEVALUE="default" # default or BNL (BNL needs this special setting)
ACCOUNTINGGROUP="none" # none or group_atlas.uiowa

# #########################################################################################################
# On what samples to run                                                                        ###########
# the entire lists of samples to do the entire analyses are defined below in two groups
# SAMPLES1 are those on which we run direct tagging with D1
# SAMPLES2 are those on which we run truth tagging with T2
# but often we want to just run on a few samples, for two possible reasons
# 1. for quick tests
# 2. Tokyo batch systems work only with one sample at a time
# we give the user the ability to do that
# By giving the command line arguments SAMPLESFINAL1 and SAMPLESFINAL2 with values other than none
# SAMPLES1 is overwritten with SAMPLESFINAL1
# SAMPLES2 is overwritten with SAMPLESFINAL2 
# below, we can overwrite the values taken from above from the command line, to say several samples for example
# #########################################################################################################

# SAMPLESFINAL1=""
# SAMPLESFINAL1+=" "
# SAMPLESFINAL1+=" WlvZqq_Sh221 ZqqZvv_Sh221 ZqqZll_Sh221"
# SAMPLESFINAL1+=" WlvZbb_Sh221 ZbbZvv_Sh221 ZbbZll_Sh221"
# SAMPLESFINAL1+=" WqqZvv_Sh221 WqqZll_Sh221 WlvZqq_Sh221" # WZ (default)
# SAMPLESFINAL1+=" ZqqZvv_Sh221 ZqqZll_Sh221 " # ZZ (default)
# SAMPLESFINAL1+=" WlvZbb_Sh221 ZbbZvv_Sh221 ZbbZll_Sh221 ggZqqZvv_Sh222 ggZqqZll_Sh222" # WZbb, ZZbb extensions and ggZH
# SAMPLESFINAL1="WlvZqq_Sh221 WlvZbb_Sh221 ZqqZvv_Sh221 ZbbZvv_Sh221"
# SAMPLESFINAL1+=" qqZvvHbbJ_PwPy8MINLO"
# SAMPLESFINAL1+=" qqZvvHbbJ_PwPy8MINLO qqZllHbbJ_PwPy8MINLO qqWlvHbbJ_PwPy8MINLO ggZvvHbb_PwPy8 ggZllHbb_PwPy8" # VH + extra jets, V->leptons, H->bb
# SAMPLESFINAL1+=" qqZvvHccJ_PwPy8MINLO qqZllHccJ_PwPy8MINLO qqWlvHccJ_PwPy8MINLO ggZvvHcc_PwPy8 ggZllHcc_PwPy8" # VH + extra jets, V->leptons, H->cc
# SAMPLESFINAL1+=" ttbar_nonallhad_PwPy8"
# SAMPLESFINAL1+=" ttbar_dilep_PwPy8"
# SAMPLESFINAL1+=" stopWt_PwPy8"
# SAMPLESFINAL1+=" stopWt_dilep_PwPy8"
# SAMPLESFINAL1+=" ttbar_nonallhad_PwPy8_METfilt"
# SAMPLESFINAL1+=" ttbar_nonallhad_PwPy8_100_200pTV ttbar_nonallhad_PwPy8_200pTV"
# SAMPLESFINAL1+=" ZnunuB_Sh221 Znunu_Sh221"
# SAMPLESFINAL1+=" ZnunuB_Sh221_PTV Znunu_Sh221_PTV"

# SAMPLESFINAL2=""
# SAMPLESFINAL2+="ZnunuC_Sh221_PTV ZnunuL_Sh221_PTV"
# SAMPLESFINAL2+="ZnunuC_Sh221_PTV"

# #########################################################################################################
# What to store: histograms and/or trees, of which type each
# #########################################################################################################

# only for histos
# if DOONLYINPUTS==true, stores only histograms of mBB and MVA to have small files for final fit
# if DOONLYINPUTS==false and DOREDUCEDHISTOS==true: store mBB/MVA + MVA inputs for post-fit plots)
# if DOONLYINPUTS==false and DOREDUCEDHISTOS==false: store all histograms

if [[ ${DO_MVA_TRAINING_TREES} == "1" ]]; then
    WRITEHISTOGRAMS="false" # separate from trees, can turn false when storing trees to save disk space
    DOONLYINPUTS="false"
    DOREDUCEDHISTOS="false" 
    USEQUANTILE="true" # true needed to store some histograms needed to do truth and hybrid continous tagging 
    GENERATESTXSSIGNALS="false" # store histos for STXS for signals (not needed for main analysis)
    DOSTOREBJETENERGYCORRHISTOS="false"
    XMLBDTS0LEP="mva mvadiboson" # also available " mvaOxford mvadibosonOxford mvaEPS mvadibosonEPS"
    # only for trees
    WRITEMVATREE="true" # both MVA and Easy trees have to be on to store the MVA training trees
    WRITEASYTREE="false" # both MVA and Easy trees have to be on to store the MVA training trees
    WRITEOSTREE="false"
    # for both histos and trees
    NOMINALONLY="true" # true to store only nominal; false to store both nominal and systematics
else
    # histograms only
    WRITEHISTOGRAMS="true" # separate from trees, can turn false when storing trees to save disk space
    DOONLYINPUTS="false"
    DOREDUCEDHISTOS="false" 
    if [[ ${USE_PCBT} == "0" ]]; then
        USEQUANTILE="false"  
    else
        USEQUANTILE="true" # true needed to store some histograms needed to do truth and hybrid continous tagging
    fi
    GENERATESTXSSIGNALS="false" # store histos for STXS for signals (not needed for main analysis)
    DOSTOREBJETENERGYCORRHISTOS="false"
    # only for trees
    WRITEMVATREE="false" # to store MVATree
    WRITEASYTREE="false" # to store EasyTree (attention: overwrites MVATree)
    WRITEOSTREE="false" # to store OSTree (attention: overwrites MVATree and EasyTree)
    # for both histos and trees
    NOMINALONLY="true" # true to store only nominal; false to store both nominal and systematics
fi

# #########################################################################################################
# How to run, issues related to physics                                                           #########
# #########################################################################################################

# DO_ICHEP_FOR_CXAODTAG31 true: reproduce ICHEP 2018 VH(bb) observation; false: state of the art analysis settings (new CDI, b-jet corrections, extensions with filtered MC)
if [[ ${DO_MVA_TRAINING_TREES} == "1" ]]; then
    DO_ICHEP_FOR_CXAODTAG31="false"
else
    DO_ICHEP_FOR_CXAODTAG31="false"
fi

USE_PF_FOR_CXAODTAG32="false" # true: use ParticleFlow; false: use EMTopo

# track-jet settings
TRACKJETCONTAINER="AntiKtVR30Rmax4Rmin02TrackJets" # variable-R default (fixed-R AntiKt2PV0TrackJets still available). Please also switch hbbVR and hbbFR.
FATJETCONTAINER="AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"
SUBJETCONTAINER="AntiKt10LCTopoTrimmedPtFrac5SmallR20ExCoM2SubJets"

if [[ ${ANASTRATEGY} == "Resolved" ]] && [[ ${ANALYSIS} == "VHbb" ]] || [[ ${ANALYSIS} == "VHcc" ]]; then
    TRACKJETCONTAINER="\"\""
    FATJETCONTAINER="\"\""
    SUBJETCONTAINER="\"\""
fi

TRACKLINKNAME="GhostVR30Rmax4Rmin02TrackJet" # fixed-R GhostAntiKt2TrackJet or variable-R (default) GhostVR30Rmax4Rmin02TrackJet
FORCETRACKJETDRMATCHING="false"
DOCOMTAGGING="false"

# b-jet energy corrections
REGULARJETCORRTYPE="PtReco" # "AllMuVR20GeVMuRElR_PtRecoTTbarBukin" # "PtReco" for R21, "PtRecoEPS" for R20
# use OneMu in VHcc for now
if [[ ${ANALYSIS} == "VHcc" ]]; then
    REGULARJETCORRTYPE="OneMu"
fi
FATJETCORRTYPE="hbbVR" # hbbVR and hbbFR are new default in v32-15. Please use xbb for v32-07.
DO_KF="true" # run KF and update main 4-vector vec_corr in 2-lepton
# FSR recovery. When it is set to true, it can only affect the 2tag with tag strategies AllSignalJets and LeadingSignalJets.
DOFSRRECOVERY="false"

# Candidate jet strategy: AllSignalJets,Leading2SignalJets,LeadingSignalJets
RESOLVEDTAGSTRATEGY="AllSignalJets"
if [[ ${ANALYSIS} == "VHcc" ]]; then
    RESOLVEDTAGSTRATEGY="Leading2SignalJets"
fi
BOOSTEDTAGSTRATEGY="Leading2SignalJets"

SPLITBOOSTEDINPUTSADDJETS="true" # Split boosted SR according to the number of additional jet in 0/1L

DONEWREGIONS="false" # split the phase space based in SR and CR on dRBB-pTV plane (resolved only)
if [[ ${ANALYSIS} == "VHbb" ]] && [[ ${ANASTRATEGY} == "Resolved" ]]; then
	DONEWREGIONS="true"
fi

# if for 1L to apply the muon trigger instead of the MET trigger
DO1LMUONTRIGGER="false" # if false it uses the MET trigger
DODYWFJCUT="true" # dY(W,H) cut - 1L boosted VHbb, default=true

# Introduces the MET trigger to the 2L analysis at a given PtZ value. Default is 150GeV
# If both are false then it the default of the muon trigger will be used.  
if [[ ${ANALYSIS} == "VHbb" ]] && [[ ${ANASTRATEGY} == "Merged" ]]; then #boosted VHbb
    DO2LMETTRIGGER="true" # This replaces the Muon trigger with the MET one at a given PtZ value for the 2L analysis
else #Allow this flag to be turned off for the Resolved VHbb and VHcc analyses.
    DO2LMETTRIGGER="false"
fi
DOMETANDMUONTRIGGER="false" # This uses the MET trigger alongside the standard on from a given PtV value for the 2L or 1L analysis.  

# how to apply or not re-weighting for additional stat samples depends on each channel, so defined for each channel

# #########################################################################################################
# SAMPLES split in SAMPLES1 (only direct tagging) and SAMPLES2 (either direct or truth tagging) ###########
# samples that are common to all channels (0L, 1L, 2L). When specific, defined below,                  ###
# for example ttbar, stop have specific extensions, and such run on different settings for each channel ###
# #########################################################################################################

# special case for Znunu for 0L e
# direct tagging
SAMPLES_ZNUNU_MAXHTPTV_1="ZnunuB_Sh221 Znunu_Sh221" # Znunu+jets with max(HT,PTV) slicing
SAMPLES_ZNUNU_PTV_1="ZnunuB_Sh221_PTV Znunu_Sh221_PTV" # Znunu+jets with PTV slicing 
# truth tagging
SAMPLES_ZNUNU_MAXHTPTV_2="ZnunuC_Sh221 ZnunuL_Sh221" # Znunu+jets with max(HT,PTV) slicing
SAMPLES_ZNUNU_PTV_AD_2="" # Znunu+jets with PTV slicing do not exist in periods a and d
SAMPLES_ZNUNU_PTV_E_2="ZnunuC_Sh221_PTV ZnunuL_Sh221_PTV" # Znunu+jets with PTV slicing exist in period e 

# SAMPLESCOMMON1 for those that are only with direct tagging
SAMPLESCOMMON1=""
# the data samples are added below for each period
SAMPLESCOMMON1+=" qqZvvHbbJ_PwPy8MINLO qqZllHbbJ_PwPy8MINLO qqWlvHbbJ_PwPy8MINLO ggZvvHbb_PwPy8 ggZllHbb_PwPy8" # VH + extra jets, V->leptons, H->bb
SAMPLESCOMMON1+=" qqZvvHccJ_PwPy8MINLO qqZllHccJ_PwPy8MINLO qqWlvHccJ_PwPy8MINLO ggZvvHcc_PwPy8 ggZllHcc_PwPy8" # VH + extra jets, V->leptons, H->cc
# SAMPLESCOMMON1+=" WincHJZZ4l_PwPy8MINLO ZincHJZZ4l_PwPy8MINLO" # VH + extra jets, V->inclusive, H->ZZ->4leptons
# SAMPLESCOMMON1+=" ggHinc_PwPy8 ggHbb_PwPy8NNLOPS bbHinc_aMCatNLOPy8 VBFHinc_PwPy8" # ggH, bbH, VBF
# SAMPLESCOMMON1+=" ttHinc_aMCatNLOPy8 ttt_MGPy8 tttt_MGPy8 ttV_aMCatNLOPy8 ttVV_MGPy8" # tt+X, X=H,t,tt,V,VV
SAMPLESCOMMON1+=" WqqZvv_Sh221 WqqZll_Sh221 WlvZqq_Sh221" # WZ (default)
SAMPLESCOMMON1+=" ZqqZvv_Sh221 ZqqZll_Sh221 " # ZZ (default)
SAMPLESCOMMON1+=" ggZqqZvv_Sh222 ggZqqZll_Sh222" # ggZZ 


# the VZbb extensions are added where it is specific to each channel, as for ICHEP 1L did not use them
# in SAMPLESCOMMON1 from V+jets it is only those that are not C or L filter (to do truth tagging a la EPS and a la Yanhui)
SAMPLESCOMMON1+=" WenuB_Sh221 Wenu_Sh221 WmunuB_Sh221 Wmunu_Sh221 WtaunuB_Sh221 Wtaunu_Sh221" # W+jets
SAMPLESCOMMON1+=" ZeeB_Sh221 Zee_Sh221 ZmumuB_Sh221 Zmumu_Sh221 ZtautauB_Sh221 Ztautau_Sh221" # Z+jets without Znunu
# the V+jets extensions are added where it is specific to each channel
# the ttbar samples default and extensions also differ per channel, so are added where it is specific to each channel

# in SAMPLES2 for those that are can be either direct or truth tagging (and the truth tagging is a la EPS and a la Yanhui)
SAMPLESCOMMON2=""
SAMPLESCOMMON2+=" WqqWlv_Sh221 " # WW (default)
SAMPLESCOMMON2+=" ggWqqWlv_Sh222" ## ggWW
SAMPLESCOMMON2+=" WenuC_Sh221 WenuL_Sh221 WmunuC_Sh221 WmunuL_Sh221 WtaunuC_Sh221 WtaunuL_Sh221" # W+jets
SAMPLESCOMMON2+=" ZeeC_Sh221 ZeeL_Sh221 ZmumuC_Sh221 ZmumuL_Sh221 ZtautauC_Sh221 ZtautauL_Sh221" # Z+jets except Znunu+jets

# now that SAMPLESCOMMON1 and SAMPLESCOMMON2 are defined, 
# for each channel we will define SAMPLES1=SAMPLESCOMMON1 and then add to that samples specific for that channel; same for 2
# we can combine them the way we need depending on the tagging required, it is done below in the for loop over ${VTAGGING}

# #########################################################################################################
# SYSTEMATICS common to all channels, more will be added for each channel (e.g. leptons not in 0L)  #######
# #########################################################################################################

# Experimental systematics
SYSTEMATICSEXPERIMENTALCOMMON=""
# split between common to all channels (e.g. PRW, jets, b-tagging, modelling
# and particularities for each channel (e.g. in 0L ignore the lepton ID systematics)
# those not 0L are commented out, so it works out of the box only for 0L for now
# PRW
SYSTEMATICSEXPERIMENTALCOMMON+=" PRW_DATASF"
# JET JVT Efficiency
SYSTEMATICSEXPERIMENTALCOMMON+=" JET_JvtEfficiency"
# FATJET (only for Boosted channel)
if [[ ${ANASTRATEGY} == "Merged" ]]; then
  SYSTEMATICSEXPERIMENTALCOMMON+=" FATJET_SubR FATJET_JER FATJET_JMR FATJET_Medium_JET_Comb_Baseline_Kin FATJET_Medium_JET_Comb_Modelling_Kin FATJET_Medium_JET_Comb_TotalStat_Kin FATJET_Medium_JET_Comb_Tracking_Kin FATJET_Medium_JET_Rtrk_Baseline_Sub FATJET_Medium_JET_Rtrk_Modelling_Sub FATJET_Medium_JET_Rtrk_TotalStat_Sub FATJET_Medium_JET_Rtrk_Tracking_Sub"
fi
# MET 
SYSTEMATICSEXPERIMENTALCOMMON+=" MET_JetTrk_Scale MET_SoftTrk_ResoPara MET_SoftTrk_ResoPerp MET_SoftTrk_Scale PH_Iso_DDonoff TRK_EFF_LOOSE_TIDE TRK_FAKE_RATE_LOOSE" # oneSidedVariations from framework-run.cfg

# Modelling systematics
if [[ ${ANALYSIS} == "VHbb" ]] && [[ ${ANASTRATEGY} == "Merged" ]]; then #boosted VHbb
    SYSTEMATICSMODELLINGCOMMON="SysVHNLOEWK SysStopWtMJ SysZMbbBoosted SysWMbbBoosted SysTTbarMJISR SysTTbarMJPS"
else #the rest
    SYSTEMATICSMODELLINGCOMMON="SysTTbarPTV SysTTbarMBB SysTTbarPtV_BDTr SysWPtV SysWPtV_BDTr SysZPtV SysWMbb SysZMbb SysTTbarPTVMBB SysStoptPTV SysStoptMBB SysStopWtPTV SysStopWtMBB SysStopWtbbACC SysStopWtothACC SysStopWtMTOP SysVVMbbME SysVVPTVME SysVVMbbPSUE SysVVPTVPSUE SysWPtVMbb SysZPtVMbb SysVHQCDscalePTV SysVHQCDscaleMbb SysVHPDFPTV SysVHQCDscalePTV_ggZH SysVHQCDscaleMbb_ggZH SysVHPDFPTV_ggZH SysVHUEPSPTV SysVHUEPSMbb SysVHNLOEWK"
fi

# Correction systematics
SYSTEMATICSCORRECTIONSCOMMON="VHNLOEWK"

# Trigger systematics
SYSTEMATICSTRIGGERCOMMON=" "

# BDT Systematics set by channel
SYSTEMATICSBDTS=" "

# #########################################################################################################
# Luminosity and Pileup Reweighting (PRW)                                                          ########  
# #########################################################################################################

# Luminosity
LUMINOSITY_1516_0L1L_ReaderVHbb="36.07456"
LUMINOSITY_1516_2L_MIA="36.2077"
LUMINOSITY_17_0L1L_ReaderVHbb="43.8"
LUMINOSITY_17_2L_MIA="43.5938"
LUMINOSITY_1516_ReaderVHbb="36.2077"
LUMINOSITY_17_ReaderVHbb="44.3074"
LUMINOSITY_18_ReaderVHbb="59.9372"

# #########################################################################################################
# Specifc booleans related to the submitReader                                                     ########
# #########################################################################################################

WRITEREADERCONFIG="false"
WRITELOGOUTPUT="false"

# #########################################################################################################
# Remove large MCEventWeights (ggZllHbb MC16d, ggZvvHbb MC16e, ggZllHcc MC16e, ggZvvHcc MC16e)     ########
# #########################################################################################################

DOLARGEMCEVENTWEIGHTSREMOVAL="true"

# #######################################################################################################################################################################
# Enable study of systematics implemented as internal weights:                                                                                                   ########
# EVTWEIGHTMODE="-1" : nominal only, does NOT store internal weight variation histograms                                                                         ########
# EVTWEIGHTMODE="0"  : stores most important internal weight variation histograms for each sample (when available)                                               ########
# EVTWEIGHTMODE="1"  : stores full set of internal weight variation histograms for each sample (when available)                                                  ########
# EVTWEIGHTMODE="2"  : stores only internal weight variation histograms for variations that are manually set by the user af part of SYSTEMATICSMODELLINGCOMMON   ########
# #######################################################################################################################################################################

EVTWEIGHTMODE="-1"

# #########################################################################################################
# Print out of chosen configuration values                                                         ########
# #########################################################################################################

echo ""
echo "DEBUG=${DEBUG}"
echo "NUMBEROFEVENTS=${NUMBEROFEVENTS}"
echo "DRIVER=${DRIVER}"
echo "BQUEUE=${BQUEUE}"
echo "CONDORQUEUE=${CONDORQUEUE}"
echo "RSEVALUE=${RSEVALUE}"
echo "ACCOUNTINGGROUP=${ACCOUNTINGGROUP}"
echo ""
echo "WRITEHISTOGRAMS=${WRITEHISTOGRAMS}"
echo "DOONLYINPUTS=${DOONLYINPUTS}"
echo "DOREDUCEDHISTOS=${DOREDUCEDHISTOS}"
echo "USEQUANTILE=${USEQUANTILE}"
echo "GENERATESTXSSIGNALS=${GENERATESTXSSIGNALS}"
echo "DOSTOREBJETENERGYCORRHISTOS=${DOSTOREBJETENERGYCORRHISTOS}"
echo "WRITEMVATREE=${WRITEMVATREE}"
echo "WRITEASYTREE=${WRITEASYTREE}"
echo "WRITEOSTREE=${WRITEOSTREE}"
echo "NOMINALONLY=${NOMINALONLY}"
echo ""
echo "DO_ICHEP_FOR_CXAODTAG31=${DO_ICHEP_FOR_CXAODTAG31}"
echo "USE_PF_FOR_CXAODTAG32=${USE_PF_FOR_CXAODTAG32}"
echo "ANASTRATEGY=${ANASTRATEGY}"
echo "TRACKJETCONTAINER=${TRACKJETCONTAINER}"
echo "FATJETCONTAINER=${FATJETCONTAINER}"
echo "SUBJETCONTAINER=${SUBJETCONTAINER}"
echo "TRACKLINKNAME=${TRACKLINKNAME}"
echo "DOCOMTAGGING=${DOCOMTAGGING}"
echo "FORCETRACKJETDRMATCHING=${FORCETRACKJETDRMATCHING}"
echo "REGULARJETCORRTYPE=${REGULARJETCORRTYPE}"
echo "FATJETCORRTYPE=${FATJETCORRTYPE}"
echo "RESOLVEDTAGSTRATEGY=${RESOLVEDTAGSTRATEGY}"
echo "BOOSTEDTAGSTRATEGY=${BOOSTEDTAGSTRATEGY}"
echo "DO1LMUONTRIGGER=${DO1LMUONTRIGGER}"
echo "DO2LMETTRIGGER=${DO2LMETTRIGGER}"
echo "DOMETANDMUONTRIGGER=${DOMETANDMUONTRIGGER}"
echo "DONEWREGIONS=${DONEWREGIONS}"
echo "DODYWFJCUT=${DODYWFJCUT}" 
echo ""
echo "SAMPLESCOMMON1=${SAMPLESCOMMON1}"
echo "SAMPLESCOMMON2=${SAMPLESCOMMON2}"
echo "SYSTEMATICSEXPERIMENTALCOMMON=${SYSTEMATICSEXPERIMENTALCOMMON}"
echo "SYSTEMATICSMODELLINGCOMMON=${SYSTEMATICSMODELLINGCOMMON}"
echo "SYSTEMATICSCORRECTIONSCOMMON=${SYSTEMATICSCORRECTIONSCOMMON}"
echo "SYSTEMATICSTRIGGERCOMMON=${SYSTEMATICSTRIGGERCOMMON}"
echo ""
echo "LUMINOSITY_1516_0L1L_ReaderVHbb=${LUMINOSITY_1516_0L1L_ReaderVHbb}"
echo "LUMINOSITY_1516_2L_MIA=${LUMINOSITY_1516_2L_MIA}"
echo "LUMINOSITY_17_0L1L_ReaderVHbb=${LUMINOSITY_17_0L1L_ReaderVHbb}"
echo "LUMINOSITY_17_2L_MIA=${LUMINOSITY_17_2L_MIA}"
echo "LUMINOSITY_1516_ReaderVHbb=${LUMINOSITY_1516_ReaderVHbb}"
echo "LUMINOSITY_17_ReaderVHbb=${LUMINOSITY_17_ReaderVHbb}"
echo "LUMINOSITY_18_ReaderVHbb=${LUMINOSITY_18_ReaderVHbb}"
echo ""
echo "WRITEREADERCONFIG=${WRITEREADERCONFIG}"
echo "DOLARGEMCEVENTWEIGHTSREMOVAL=${DOLARGEMCEVENTWEIGHTSREMOVAL}"
echo "SPLITBOOSTEDINPUTSADDJETS=${SPLITBOOSTEDINPUTSADDJETS}"
echo ""

# #########################################################################################################
# Loop over the channels, mctype, vtag, tagging, for each update the configs with their specific and run ##
# #########################################################################################################

mkdir -p ${OUTPUT_FOLDER}
CONFIG_INITIAL_STEM="data/CxAODReader_VHbb/framework-read-automatic"
CONFIG_INITIAL="${WorkDir_DIR}/${CONFIG_INITIAL_STEM}.cfg"
echo "CONFIG_INITIAL_STEM=${CONFIG_INITIAL_STEM}"
echo "CONFIG_INITIAL=${CONFIG_INITIAL}"
echo ""
echo "Assume we are in the run folder, and start loop over VTAG!"

# in order to submit at the same time 31-10 and 32-07 we need to loop over VTAG first
# from the VTAG name set automatically the CXAODTAG
# and if CxAODTag31, use the DO_ICHEP seeting desired, if CxAODTag32,set DO_ICHEP=false
# so need to move in here all settings that depend on DO_ICHEP
for VTAG in `echo "${VTAGs}" | awk -v RS=, '{print}'`
do
    echo ""
    echo "************************************************"
    echo "**** VTAG=${VTAG} ****"
    echo "************************************************"

    # from the VTAG, e.g. 32-07-P, find the version, by taking what is before the first 
    VTAG_VERSION="${VTAG%%-*}"    
    echo "VTAG_VERSION=${VTAG_VERSION}"
    # set the CXAODTAG based on VTAG_VERSION
    CXAODTAG="CxAODTag${VTAG_VERSION}"
    echo "CXAODTAG=${CXAODTAG}"

    # if you want to optimize for each sample so that the total CxAOD file per job, set JOBSIZELIMITMB, and NRFILESPERJOB will be ignored (e.g. -1 to show it is ignored)
    # if you want to give a number of files per job to be used for all samples, set JOBSIZELIMITMB to negative (e.g. -1) set NRFILESPERJOB >= 1
    # the size per event of 31-10 is much larger than 32-07 by factor of 2 due to size reduction in 32-xx and by a factor of 4 due to systematics in 31-10
    # so it is best to optimise the job size limit as a function of the CxAODTag

    # as a function of CXAODTAG, we set DO_ICHEP and USE_PF
    # for DO_ICHEP: if CxAODTag32, DO_ICHEP is only false, if CxAODTag31, it can be either true or false.
    # for USE_PF: if CxAODTag31, USE_PF is only false, if CxAODTag32, it can be either true or false.    
    if [[ ${CXAODTAG} == "CxAODTag31" ]]; then
	DO_ICHEP="${DO_ICHEP_FOR_CXAODTAG31}"
	USE_PF="false"
	JOBSIZELIMITMB="3000"
	NRFILESPERJOB="-1"
        # JES and JER SYS for CxAODTag31
        SYSTEMATICSEXPERIMENTALCOMMON+=" JET_JER_SINGLE_NP JET_23NP_JET_BJES_Response JET_23NP_JET_EffectiveNP_1 JET_23NP_JET_EffectiveNP_2 JET_23NP_JET_EffectiveNP_3 JET_23NP_JET_EffectiveNP_4 JET_23NP_JET_EffectiveNP_5 JET_23NP_JET_EffectiveNP_6 JET_23NP_JET_EffectiveNP_7 JET_23NP_JET_EffectiveNP_8restTerm JET_23NP_JET_EtaIntercalibration_Modelling JET_23NP_JET_EtaIntercalibration_NonClosure_highE JET_23NP_JET_EtaIntercalibration_NonClosure_negEta JET_23NP_JET_EtaIntercalibration_NonClosure_posEta JET_23NP_JET_EtaIntercalibration_TotalStat JET_23NP_JET_Flavor_Composition JET_23NP_JET_Flavor_Response JET_23NP_JET_Pileup_OffsetMu JET_23NP_JET_Pileup_OffsetNPV JET_23NP_JET_Pileup_PtTerm JET_23NP_JET_Pileup_RhoTopology JET_23NP_JET_PunchThrough_MC16 JET_23NP_JET_SingleParticle_HighPt"
	# below cuts not done at Maker level for CxAODTag31
	# IMPORTANT: The flags DOLOWERPTZ2LCUT and DONJETCUT0L1L have to be consistent in both the Maker and Reader config
	DOLOWERPTZ2LCUT="false"
	DONJETCUT0L1L="false"
    elif [[ ${CXAODTAG} == "CxAODTag32" ]]; then
	DO_ICHEP="false"
	USE_PF="${USE_PF_FOR_CXAODTAG32}"
	if [ ${NOMINALONLY} == "true" ]; 
	then
	    JOBSIZELIMITMB="2000"
	else
	    JOBSIZELIMITMB="4000"
	fi
	NRFILESPERJOB="-1"
        # JET full set systematics Category Reduction
        SYSTEMATICSEXPERIMENTALCOMMON+=" JET_CR_JET_BJES_Response JET_CR_JET_EffectiveNP_Detector1 JET_CR_JET_EffectiveNP_Detector2 JET_CR_JET_EffectiveNP_Mixed1 JET_CR_JET_EffectiveNP_Mixed2 JET_CR_JET_EffectiveNP_Mixed3 JET_CR_JET_EffectiveNP_Modelling1 JET_CR_JET_EffectiveNP_Modelling2 JET_CR_JET_EffectiveNP_Modelling3 JET_CR_JET_EffectiveNP_Modelling4 JET_CR_JET_EffectiveNP_Statistical1 JET_CR_JET_EffectiveNP_Statistical2 JET_CR_JET_EffectiveNP_Statistical3 JET_CR_JET_EffectiveNP_Statistical4 JET_CR_JET_EffectiveNP_Statistical5 JET_CR_JET_EffectiveNP_Statistical6 JET_CR_JET_EtaIntercalibration_Modelling JET_CR_JET_EtaIntercalibration_NonClosure_highE JET_CR_JET_EtaIntercalibration_NonClosure_negEta JET_CR_JET_EtaIntercalibration_NonClosure_posEta JET_CR_JET_EtaIntercalibration_TotalStat JET_CR_JET_Flavor_Composition JET_CR_JET_Flavor_Response JET_CR_JET_JER_DataVsMC JET_CR_JET_JER_EffectiveNP_1 JET_CR_JET_JER_EffectiveNP_2 JET_CR_JET_JER_EffectiveNP_3 JET_CR_JET_JER_EffectiveNP_4 JET_CR_JET_JER_EffectiveNP_5 JET_CR_JET_JER_EffectiveNP_6 JET_CR_JET_JER_EffectiveNP_7restTerm JET_CR_JET_Pileup_OffsetMu JET_CR_JET_Pileup_OffsetNPV JET_CR_JET_Pileup_PtTerm JET_CR_JET_Pileup_RhoTopology JET_CR_JET_PunchThrough_MC16 JET_CR_JET_SingleParticle_HighPt"
	# VHbb specific preselection cuts done at Maker level for CxAODTag32
	# IMPORTANT: The flags DOLOWERPTZ2LCUT and DONJETCUT0L1L have to be consistent in both the Maker and Reader config
	DOLOWERPTZ2LCUT="true"
	DONJETCUT0L1L="true"
    else
	echo "The CxAODTag you chose is not defined and supported. Choose CxAODTag31 or CxAODTag32! Will ABORT!!!"
	exit 1
    fi
    # echo the values DO_ICHEP and USE_PF
    if [[ ${DO_ICHEP} == "true" ]]; then 
	echo "DO_ICHEP=true (VHbb discovery from ICHEP 2018)"
    else # this is to do the state of the art latest versions
	echo "DO_ICHEP=false (state of the art)"
    fi
    echo "USE_PF=${USE_PF}"
    echo "JOBSIZELIMITMB=${JOBSIZELIMITMB}"
    echo "NRFILESPERJOB=${NRFILESPERJOB}"
    echo "DOLOWERPTZ2LCUT=${DOLOWERPTZ2LCUT}"
    echo "DONJETCUT0L1L=${DONJETCUT0L1L}"
    # MVA settings
    READMVA="false"
    # evaluate ICHEP MVA (always true if DO_ICHEP is true)
    if [[ ${DO_ICHEP} == "true" ]]; then 
	ICHEPMVA="true"
    else
	ICHEPMVA="false"
    fi
    echo "READMVA=${READMVA}"
    echo "ICHEPMVA-${ICHEPMVA}"

    # depending on USE_PF
    # regular jet choice
    if [ ${USE_PF} == "true" ]; then
        # ParticleFlow (PF)
	REGULARJETCONTAINER="AntiKt4EMPFlowJets"
	METCONTAINER="MET_Reference_AntiKt4EMPFlow"
    else
        # EMTopo
	REGULARJETCONTAINER="AntiKt4EMTopoJets"
	METCONTAINER="MET_Reference_AntiKt4EMTopo"
    fi
    echo "REGULARJETCONTAINER=${REGULARJETCONTAINER}"
    echo "METCONTAINER=${METCONTAINER}"

    # kinematic fit configuration
    if [[ ${DO_ICHEP} == "true" ]]; then
        KF_CONFIG="2017"
    else
        KF_CONFIG="2019b"
    fi

    if [[ ${ANALYSIS} == "VHcc" ]]; then
        KF_CONFIG="2019c"
    fi

    # b-tagging CDI
    if [[ ${DO_ICHEP} == "true" ]]; then 
	BTAGGINGCDIFILE="2017-21-13TeV-MC16-CDI-2018-05-04_v1" # used for ICHEP result
	# BTAGGINGCDIFILE="2017-21-13TeV-MC16-CDI-2018-10-19_v1" # temporary new CDI
    elif [[ ${ANASTRATEGY} == "Merged" ]]; then
	BTAGGINGCDIFILE="2017-21-13TeV-MC16-CDI-2019-07-30_v1"
    else 
#	BTAGGINGCDIFILE="2017-21-13TeV-MC16-CDI-2018-10-19_v1"
#	BTAGGINGCDIFILE="/afs/cern.ch/user/i/iluise/public/2019-13TeV_Feb19_CDI-2019-02-18_v1_Continuous.root"
    BTAGGINGCDIFILE="/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/FullRunII2019/2017-21-13TeV-MC16-CDI-2019-07-30_v1_CustomMaps.root"
    fi
    echo "BTAGGINGCDIFILE=${BTAGGINGCDIFILE}"

    # need to currently use a different CDI for VHcc - can be removed soon hopefully
    if [[ ${ANALYSIS} == "VHcc" ]]; then
	BTAGGINGCDIFILE="/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VHcc/CDI/2017-21-13TeV-MC16-CDI-November4-2019.root"
    fi
    echo "BTAGGINGCDIFILE=${BTAGGINGCDIFILE}"
    
    # b/c-tagging operating point
    if [[ ${USE_PCBT} == "0" ]]; then
        BTAGGINGCONFIGS="MV2c10 70 AntiKt4EMTopoJets FixedCut"
    else
        BTAGGINGCONFIGS="MV2c10 70 AntiKt4EMTopoJets Continuous"
    fi	
    if [[ ${ANALYSIS} == "VHcc" ]]; then
	BTAGGINGCONFIGS="DL1 Tight_Veto_MV2c10_FixedCutBEff_70 AntiKt4EMTopoJets CTag"
    fi
    echo "BTAGGINGCONFIGS=${BTAGGINGCONFIGS}"

    # b-tagging weight systematics (#BTAGLOOSE, BTAGMEDIUM, BTAGTIGHT)
    if [[ ${DO_ICHEP} == "true" ]]; then 
	SYSTEMATICSWEIGHTCOMMON="BTAGMEDIUM"
    else  # this is to do the state of the art latest versions
	SYSTEMATICSWEIGHTCOMMON="BTAGLOOSE"
    fi 
    echo "SYSTEMATICSWEIGHTCOMMON=${SYSTEMATICSWEIGHTCOMMON}"

    if [[ ${USE_PCBT} == "0" ]]; then
        EXCLUDEDBTAGEIGENVECTORS="none"
    else
        EXCLUDEDBTAGEIGENVECTORS="FT_EFF_Eigen_B_3__1up;FT_EFF_Eigen_B_6__1up;FT_EFF_Eigen_B_11__1up;FT_EFF_Eigen_B_13__1up;FT_EFF_Eigen_B_20__1up;FT_EFF_Eigen_B_27__1up;FT_EFF_Eigen_B_28__1up;FT_EFF_Eigen_B_29__1up;FT_EFF_Eigen_B_30__1up;FT_EFF_Eigen_B_33__1up;FT_EFF_Eigen_B_34__1up;FT_EFF_Eigen_B_35__1up;FT_EFF_Eigen_B_41__1up;FT_EFF_Eigen_B_42__1up;FT_EFF_Eigen_B_43__1up;FT_EFF_Eigen_B_44__1up;FT_EFF_Eigen_C_11__1up;FT_EFF_Eigen_C_16__1up;FT_EFF_Eigen_Light_10__1up;FT_EFF_Eigen_Light_11__1up;FT_EFF_Eigen_Light_12__1up;FT_EFF_Eigen_Light_13__1up;FT_EFF_Eigen_Light_14__1up;FT_EFF_Eigen_Light_15__1up;FT_EFF_Eigen_Light_16__1up;FT_EFF_Eigen_Light_17__1up;FT_EFF_Eigen_Light_18__1up;FT_EFF_Eigen_Light_19__1up;"
    fi
    echo "EXCLUDEDBTAGEIGENVECTORS=${EXCLUDEDBTAGEIGENVECTORS}"

    # in case we want to redo the PRW at the Reader level, which we prefer not to, however, as it is not 100% identical with that at Maker level
    # lumi calc for PRW
    ILUMICALCFILES_1516="GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root"
    if [[ ${DO_ICHEP} == "true" ]]; then 
	ILUMICALCFILES_17="GoodRunsLists/data17_13TeV/20180309/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root"
    else # this is to do the state of the art latest versions
	ILUMICALCFILES_17="GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root"  
    fi
    ILUMICALCFILES_18="GoodRunsLists/data18_13TeV/20181111/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-001.root"
    echo "ILUMICALCFILES_1516=${ILUMICALCFILES_1516}"
    echo "ILUMICALCFILES_17=${ILUMICALCFILES_17}"
    echo "ILUMICALCFILES_18=${ILUMICALCFILES_18}"

    # Config files for PRW
    CONFIGFILES_a="${WorkDir_DIR}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16a.root" # with averageMu
    CONFIGFILES_c="${WorkDir_DIR}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16c.root" # with averageMu
    if [[ ${DO_ICHEP} == "true" ]]; then 
	CONFIGFILES_d="${WorkDir_DIR}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16d.root GoodRunsLists/data17_13TeV/20180309/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root" # with actualMu
    else # this is to do the state of the art latest versions
	CONFIGFILES_d="${WorkDir_DIR}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16d.root GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root" # with actualMu  
    fi
    CONFIGFILES_e="${WorkDir_DIR}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16e.root" # with averageMu
    echo "CONFIGFILES_a=${CONFIGFILES_a}"
    echo "CONFIGFILES_d=${CONFIGFILES_d}"
    echo "CONFIGFILES_e=${CONFIGFILES_e}"
    echo "For VTAG=${VTAG} start looping over CHANNEL" 

    # loop over channels
    for CHANNEL in `echo "${CHANNELs}" | awk -v RS=, '{print}'`
    do
	echo ""
	echo "*** VTAG=${VTAG} CHANNEL=${CHANNEL} ***"
	SAMPLESCHANNEL1="${SAMPLESCOMMON1}"
	SAMPLESCHANNEL2="${SAMPLESCOMMON2}"
        #
	if [[ "${CHANNEL}" == "0L" ]]; then
            # #####################################################################################################
	    # 0L CHANNEL                                                                                      #####
            # #####################################################################################################
	    LEPCHANNEL="0lep"
	    DERIVATION="HIGG5D1"
	    # LUMINOSITY
	    if [[ ${DO_ICHEP} == "true" ]]; then 
		LUMINOSITY_1516="${LUMINOSITY_1516_0L1L_ReaderVHbb}"
		LUMINOSITY_17="${LUMINOSITY_17_0L1L_ReaderVHbb}"
	    else # this is to do the state of the art latest versions
		LUMINOSITY_1516="${LUMINOSITY_1516_ReaderVHbb}"
		LUMINOSITY_17="${LUMINOSITY_17_ReaderVHbb}"
	    fi
	    LUMINOSITY_18="${LUMINOSITY_18_ReaderVHbb}"
	    # merge 3pjet in histograms
	    DOMERGEJETBINS="false"
	    # SAMPLESCHANNEL1 
	    # ttbar
	    SAMPLESCHANNEL1+=" ttbar_nonallhad_PwPy8" # ttbar default sample
	    USETTBARMETFILTERED0LEPSAMPLES="true" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
	    SAMPLESCHANNEL1+=" ttbar_nonallhad_PwPy8_METfilt" # ttbar nonallhad MET filtered to be reweighted and added with the re-weight ttbar nonallhad that was the default
	    # stop
	    SAMPLESCHANNEL1+=" stops_PwPy8"  # stops
	    SAMPLESCHANNEL1+=" stopt_PwPy8"  # stopt
	    SAMPLESCHANNEL1+=" stopWt_PwPy8" # stopWt
            # SAMPLESCHANNEL1+=" stoptZ_MGPy8" # stoptZ
	    # diboson extensions
	    # APPLYVZBBWEIGHT="None" # running only the default and the extensions are set to zero
	    APPLYVZBBWEIGHT="Weight" # "Weight"  means diboson VZbb filtering for all channels with reweight.
	    # in both cases we add the diboson extensions and ggZH
	    SAMPLESCHANNEL1+=" WlvZbb_Sh221 ZbbZvv_Sh221 ZbbZll_Sh221" # VZbb filtered via re-weighted qq->VV
	    # ggVV depend on period, but same for all channels
	    # other extensions
	    DOREMOVEDILEPOVERLAP="false" # for ttbar and stopWt
	    USETTBARPTWFILTERED1LEPSAMPLES="false" # only does something for 1lep selection, and nothing for the others (0lep, 2lep)
	    # SYSTEMATICS
	    SYSTEMATICSEXPERIMENTAL="${SYSTEMATICSEXPERIMENTALCOMMON}"
	    SYSTEMATICSMODELLING="${SYSTEMATICSMODELLINGCOMMON}"
	    SYSTEMATICSCORRECTIONS="${SYSTEMATICSCORRECTIONSCOMMON}"
	    SYSTEMATICSWEIGHT="${SYSTEMATICSWEIGHTCOMMON}"
	    SYSTEMATICSTRIGGER="${SYSTEMATICSTRIGGERCOMMON}"
	    # MET trigger
	    SYSTEMATICSTRIGGER+=" METTrigStat METTrigTop METTrigZ METTrigSumpt"
      # BDT Systematics
      SYSTEMATICSBDTS+="ttbar_ME ttbar_PS W_SHtoMG5"
	    DOPTVSPLITTING250GEV="false"
	    #Additional VHbb and VHcc 0lep Configuration
		if [[ ${ANALYSIS} == "VHbb" ]] || [[ ${ANALYSIS} == "VHcc" ]]; then
			DOPTVSPLITTING250GEV="true"
		fi	
# done 0L
	elif [[ "${CHANNEL}" == "1L" ]]; then
            # #####################################################################################################
	    # 1L CHANNEL                                                                                      #####
            # #####################################################################################################
	    LEPCHANNEL="1lep"
	    DERIVATION="HIGG5D2"
	    # LUMINOSITY
	    if [[ ${DO_ICHEP} == "true" ]]; then 
		LUMINOSITY_1516="${LUMINOSITY_1516_0L1L_ReaderVHbb}"
		LUMINOSITY_17="${LUMINOSITY_17_0L1L_ReaderVHbb}"
	    else # this is to do the state of the art latest versions
		LUMINOSITY_1516="${LUMINOSITY_1516_ReaderVHbb}"
		LUMINOSITY_17="${LUMINOSITY_17_ReaderVHbb}"
	    fi
	    LUMINOSITY_18="${LUMINOSITY_18_ReaderVHbb}"
	    # merge 3pjet in histograms
	    DOMERGEJETBINS="false"
	    # SAMPLESCHANNEL1
	    if [[ ${DO_ICHEP} == "true" ]]; then
		SAMPLESCHANNEL1+=" ttbar_nonallhad_PwPy8" # ttbar (nonallhad - default)
		SAMPLESCHANNEL1+=" stopWt_PwPy8" # stopWt (default)
		DOREMOVEDILEPOVERLAP="false" # for ttbar and stopWt
		USETTBARPTWFILTERED1LEPSAMPLES="false" # only does something for 1lep selection, and nothing for the others (0lep, 2lep)
		# since false, nothing to add
	    else
	        # state of the art
		SAMPLESCHANNEL1+=" ttbar_nonallhad_PwPy8" # ttbar (nonallhad - default)
		SAMPLESCHANNEL1+=" stopWt_PwPy8" # stopWt (default)
		DOREMOVEDILEPOVERLAP="true" # for ttbar and stopWt
		SAMPLESCHANNEL1+=" ttbar_dilep_PwPy8" # ttbar dilep extension
		SAMPLESCHANNEL1+=" stopWt_dilep_PwPy8" # stopWt dilep extension
		USETTBARPTWFILTERED1LEPSAMPLES="true" # only does something for 1lep selection, and nothing for the others (0lep, 2lep)
		SAMPLESCHANNEL1+=" ttbar_nonallhad_PwPy8_100_200pTV ttbar_nonallhad_PwPy8_200pTV" # ttbar ( extensions with pTW filter)
		# USETTBARPTWFILTERED1LEPSAMPLES defined for 1L in the loop over periods since
		# the ttbar_pTW extension varies with period, as samples do not exist yet in period e
	    fi
	    # the rest of stop
	    SAMPLESCHANNEL1+=" stops_PwPy8"  # stops
	    SAMPLESCHANNEL1+=" stopt_PwPy8"  # stopt
            # SAMPLESCHANNEL1+=" stoptZ_MGPy8" # stoptZ
	    # diboson extensions
	    if [[ ${DO_ICHEP} == "true" ]]; then
		echo "ICHEP 1L no VZbb filtered samples"
		APPLYVZBBWEIGHT="None" # "None" means ignore completely the bb-filtered samples and use only the inclusive samples
		# and no need then to run over the extended samples, and neither the ggZH
	    else
	        # state of the art
		# APPLYVZBBWEIGHT="None" # running only the default and the extensions are set to zero
		APPLYVZBBWEIGHT="Weight" # "Weight"  means diboson VZbb filtering for all channels with reweight, so add the samples
		SAMPLESCHANNEL1+=" WlvZbb_Sh221 ZbbZvv_Sh221 ZbbZll_Sh221" # VZbb filtered via re-weighted qq->VV
		# ggVV depend on period, but same for all channels
	    fi
	    # other extensions
	    USETTBARMETFILTERED0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
            # SYSTEMATICS
	    SYSTEMATICSEXPERIMENTAL="${SYSTEMATICSEXPERIMENTALCOMMON}"
	    SYSTEMATICSMODELLING="${SYSTEMATICSMODELLINGCOMMON}"
	    SYSTEMATICSCORRECTIONS="${SYSTEMATICSCORRECTIONSCOMMON}"
	    SYSTEMATICSWEIGHT="${SYSTEMATICSWEIGHTCOMMON}"
	    SYSTEMATICSTRIGGER="${SYSTEMATICSTRIGGERCOMMON}"
            # ELECTRON energy scale/resolution (not used in 0L)
	    SYSTEMATICSEXPERIMENTAL+=" EG_RESOLUTION_ALL EG_SCALE_ALL"
	    # ELECTRON ID efficiency (not useed in 0L)
	    SYSTEMATICSEXPERIMENTAL+=" EL_EFF_ID_TOTAL_1NPCOR_PLUS_UNCOR EL_EFF_Iso_TOTAL_1NPCOR_PLUS_UNCOR EL_EFF_Reco_TOTAL_1NPCOR_PLUS_UNCOR EL_EFF_Trigger_TOTAL_1NPCOR_PLUS_UNCOR"
	    # MUON momentum scale/resolution (not used in 0L)
	    SYSTEMATICSEXPERIMENTAL+=" MUON_ID MUON_MS MUON_SAGITTA_RESBIAS MUON_SAGITTA_RHO MUON_SCALE"
	    # MUON ID efficiency (not used in 0L)
	    SYSTEMATICSEXPERIMENTAL+=" MUON_EFF_ISO_STAT MUON_EFF_ISO_SYS MUON_EFF_RECO_STAT MUON_EFF_RECO_STAT_LOWPT MUON_EFF_RECO_SYS MUON_EFF_RECO_SYS_LOWPT MUON_EFF_TTVA_STAT MUON_EFF_TTVA_SYS PH_EFF_ID_Uncertainty"
	    # TAU scale/resolution and ID
	    SYSTEMATICSEXPERIMENTAL+=" TAUS_TRUEHADTAU_SME_TES_DETECTOR TAUS_TRUEHADTAU_SME_TES_INSITU TAUS_TRUEHADTAU_SME_TES_MODEL" # variations from framework-run.cfg
	    # MUON trigger 
	    if [[ ${DO1LMUONTRIGGER} == "false" ]]; then
            SYSTEMATICSTRIGGER+=" MUON_EFF_TrigSystUncertainty MUON_EFF_TrigStatUncertainty METTrigStat METTrigTop METTrigZ METTrigSumpt"
            else 
            SYSTEMATICSTRIGGER+=" MUON_EFF_TrigSystUncertainty MUON_EFF_TrigStatUncertainty"
            fi

      # BDT Systematics
      SYSTEMATICSBDTS+="ttbar_ME ttbar_PS W_SHtoMG5"
      # if ICHEP 2018 CxAODs, use variable-R track jets, enforce dR matching with fat-jets and apply no muon-in-jet correction
	    if [[ ${CXAODTAG} == "CxAODTag31" ]]; then
		TRACKJETCONTAINER="AntiKtVR30Rmax4Rmin02TrackJets"
		TRACKLINKNAME="GhostVR30Rmax4Rmin02TrackJet"
		FORCETRACKJETDRMATCHING="true"
	    fi
	    DOPTVSPLITTING250GEV="false"
		
		#Additional VHcc1lep Configuration
		#Remove 0lepHbb sample from VHcc1lep running
		if [[ ${ANALYSIS} == "VHcc" ]]; then
		SAMPLESCOMMON1=${SAMPLESCOMMON1//qqZvvHbbJ_PwPy8MINLO/ }
		SAMPLESCOMMON1=${SAMPLESCOMMON1//ggZvvHbb_PwPy8/ }
		SAMPLESCHANNEL1=${SAMPLESCHANNEL1//qqZvvHbbJ_PwPy8MINLO/ }
		SAMPLESCHANNEL1=${SAMPLESCHANNEL1//ggZvvHbb_PwPy8/ }
		fi
		if [[ ${ANALYSIS} == "VHbb" ]] || [[ ${ANALYSIS} == "VHcc" ]]; then
			DOPTVSPLITTING250GEV="true"
		fi		
	    # done 1L
	elif [[ "${CHANNEL}" == "2L" ]]; then
            # #####################################################################################################
	    # 2L CHANNEL                                                                                      #####
            # #####################################################################################################
	    LEPCHANNEL="2lep"
	    DERIVATION="HIGG2D4"
	    # LUMINOSITY
	    if [[ ${DO_ICHEP} == "true" ]]; then 
		LUMINOSITY_1516="${LUMINOSITY_1516_2L_MIA}"
		LUMINOSITY_17="${LUMINOSITY_17_2L_MIA}"
	    else # this is to do the state of the art latest versions
		LUMINOSITY_1516="${LUMINOSITY_1516_ReaderVHbb}"
		LUMINOSITY_17="${LUMINOSITY_17_ReaderVHbb}"
	    fi
	    LUMINOSITY_18="${LUMINOSITY_18_ReaderVHbb}"
	    # merge 3pjet in histograms
	    DOMERGEJETBINS="true"
	    # SAMPLESCHANNEL1
	    SAMPLESCHANNEL1+=" ttbar_dilep_PwPy8" # ttbar dilep (default for 2L)
	    SAMPLESCHANNEL1+=" stopWt_dilep_PwPy8" # stopWt dilep (default for 2L)
	    DOREMOVEDILEPOVERLAP="true" # for ttbar and stopWt
	    SAMPLESCHANNEL1+=" ttbar_nonallhad_PwPy8" # ttbar nonallhad for dilep extension in 2L (doesn't change 2jet, but adds more in 3pjet)
	    if [[ ${DO_ICHEP} == "true" ]]; then # This is needed to reproduce ICHEP
		echo "DO_ICHEP=true, skip stopWt_PwPy8 sample for 2-lepton"
	    else
		SAMPLESCHANNEL1+=" stopWt_PwPy8" # stopWt inclusive extension (basically doesn't change distributions)
	    fi
	    # other stop
	    SAMPLESCHANNEL1+=" stops_PwPy8"  # stops
	    SAMPLESCHANNEL1+=" stopt_PwPy8"  # stopt
            # SAMPLESCHANNEL1+=" stoptZ_MGPy8" # stoptZ
	    # diboson extensions
	    if [[ ${DO_ICHEP} == "true" ]]; then
		APPLYVZBBWEIGHT="Veto" # "Veto" means ignore VZbb from the inclusive sample and take only from the bb-filtred sample
	    else
		# APPLYVZBBWEIGHT="None" # running only the default and the extensions are set to zero
		APPLYVZBBWEIGHT="Weight" # "Weight"  means diboson VZbb filtering for all channels with reweight.
	    fi
	    # in both cases we add the diboson extensions and ggZH
	    SAMPLESCHANNEL1+=" WlvZbb_Sh221 ZbbZvv_Sh221 ZbbZll_Sh221" # VZbb filtered via re-weighted qq->VV
	    # ggVV depend on period, but same for all channels
	    # other extensions
	    USETTBARMETFILTERED0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
	    USETTBARPTWFILTERED1LEPSAMPLES="false" # only does something for 1lep selection, and nothing for the others (0lep, 2lep)
            # SYSTEMATICS
	    SYSTEMATICSEXPERIMENTAL="${SYSTEMATICSEXPERIMENTALCOMMON}"
	    SYSTEMATICSMODELLING="${SYSTEMATICSMODELLINGCOMMON}"
	    SYSTEMATICSCORRECTIONS="${SYSTEMATICSCORRECTIONSCOMMON}"
	    SYSTEMATICSWEIGHT="${SYSTEMATICSWEIGHTCOMMON}"
	    SYSTEMATICSTRIGGER="${SYSTEMATICSTRIGGERCOMMON}"
            # ELECTRON energy scale/resolution (not used in 0L)
	    SYSTEMATICSEXPERIMENTAL+=" EG_RESOLUTION_ALL EG_SCALE_ALL"
	    # ELECTRON ID efficiency (not useed in 0L)
	    SYSTEMATICSEXPERIMENTAL+=" EL_EFF_ID_TOTAL_1NPCOR_PLUS_UNCOR EL_EFF_Iso_TOTAL_1NPCOR_PLUS_UNCOR EL_EFF_Reco_TOTAL_1NPCOR_PLUS_UNCOR EL_EFF_Trigger_TOTAL_1NPCOR_PLUS_UNCOR"
	    # MUON momentum scale/resolution (not used in 0L)
	    SYSTEMATICSEXPERIMENTAL+=" MUON_ID MUON_MS MUON_SAGITTA_RESBIAS MUON_SAGITTA_RHO MUON_SCALE"
	    # MUON ID efficiency (not used in 0L)
	    SYSTEMATICSEXPERIMENTAL+=" MUON_EFF_ISO_STAT MUON_EFF_ISO_SYS MUON_EFF_RECO_STAT MUON_EFF_RECO_STAT_LOWPT MUON_EFF_RECO_SYS MUON_EFF_RECO_SYS_LOWPT MUON_EFF_TTVA_STAT MUON_EFF_TTVA_SYS PH_EFF_ID_Uncertainty"
	    # TAU scale/resolution and ID
	    SYSTEMATICSEXPERIMENTAL+=" TAUS_TRUEHADTAU_SME_TES_DETECTOR TAUS_TRUEHADTAU_SME_TES_INSITU TAUS_TRUEHADTAU_SME_TES_MODEL" # variations from framework-run.cfg
	    # MUON trigger
	    SYSTEMATICSTRIGGER+=" MUON_EFF_TrigSystUncertainty MUON_EFF_TrigStatUncertainty"
	    #MET trigger is not used on 2L (for now)
	    DOPTVSPLITTING250GEV="false"

		#Additional VHcc2lep Configuration
		if [[ ${ANALYSIS} == "VHbb" ]] || [[ ${ANALYSIS} == "VHcc" ]]; then
			DOPTVSPLITTING250GEV="true"
		fi	
	    # done 2L
	else
	    echo "CHANNEL=${CHANNEL} not known. Choose 0L, 1L or 2L. Will ABORT!!!"
	    exit 1
	fi
        # done if CHANNEL is 0L, 1L or 2L
	echo "SAMPLESCHANNEL1=${SAMPLESCHANNEL1}"
	echo "SAMPLESCHANNEL2=${SAMPLESCHANNEL2}"
	echo "LEPCHANNEL=${LEPCHANNEL}"
	echo "DERIVATION=${DERIVATION}"
	echo "LUMINOSITY_1516=${LUMINOSITY_1516}"
	echo "LUMINOSITY_17=${LUMINOSITY_17}"
	echo "LUMINOSITY_18=${LUMINOSITY_18}"
	echo "DOMERGEJETBINS=${DOMERGEJETBINS}"
	echo "SAMPLESCHANNEL1=${SAMPLESCHANNEL1}"
	echo "SAMPLESCHANNEL2=${SAMPLESCHANNEL2}"
	echo "APPLYVZBBWEIGHT=${APPLYVZBBWEIGHT}"
	echo "DOREMOVEDILEPOVERLAP=${DOREMOVEDILEPOVERLAP}"
	echo "USETTBARMETFILTERED0LEPSAMPLES=${USETTBARMETFILTERED0LEPSAMPLES}"
	echo "SYSTEMATICSEXPERIMENTAL=${SYSTEMATICSEXPERIMENTAL}"
	echo "SYSTEMATICSMODELLING=${SYSTEMATICSMODELLING}"
	echo "SYSTEMATICSCORRECTIONS=${SYSTEMATICSCORRECTIONS}"
	echo "SYSTEMATICSWEIGHT=${SYSTEMATICSWEIGHT}"
	echo "SYSTEMATICSTRIGGER=${SYSTEMATICSTRIGGER}"
  echo "SYSTEMATICSBDTS=${SYSTEMATICSBDTS}"
	echo "DOPTVSPLITTING250GEV=${DOPTVSPLITTING250GEV}"
  echo "DO_KF"=${DO_KF}
  echo "KF_CONFIG"=${KF_CONFIG}
  echo "DOFSRRECOVERY"=${DOFSRRECOVERY}
	echo "For VTAG=${VTAG} and CHANNEL=${CHANNEL} start loop over MCTYPE"
        # loop over MCTYPE (a, c, d)
	for MCTYPE in `echo "${MCTYPEs}" | awk -v RS=, '{print}'`
	do
	    echo ""
	    echo "*** VTAG=${VTAG} CHANNEL=${CHANNEL} MCTYPE=${MCTYPE} ***"
	    SAMPLESMCTYPE1="${SAMPLESCHANNEL1}"
	    SAMPLESMCTYPE2="${SAMPLESCHANNEL2}"
	    if [[ "${MCTYPE}" == "a" ]]; then
		LUMINOSITY="${LUMINOSITY_1516}"
		ILUMICALCFILES="${ILUMICALCFILES_1516}"
		CONFIGFILES="${CONFIGFILES_a}"
	        # Note that for ICHEP 2018 derivations one of the samples in period a for Znunu_PTV extensions was missing at derivation level
	        # so this extension could not be used
	        # in mc16d they are complete, and can be used for testing, but in real analysis things must be consistent
	        # so turning mc16d also to false
		SAMPLESMCTYPE1+=" data15 data16"
		# ZNUNU
		if [[ "${CHANNEL}" == "0L" ]]; then
		    if [[ "${CXAODTAG}" == "CxAODTag31" ]]; then
			# Sh_221_NN30NNLO_Znunu_PTV140_280_MJJ0_500_BFilter) having 10% of events in period a than in period d
			# due to that we can not use the extension, it actually gives worse stats, even if same yield and shape
			USEZNUNUEXTENSION0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
			SAMPLES_ZNUNU_1="${SAMPLES_ZNUNU_MAXHTPTV_1}"
			SAMPLES_ZNUNU_2="${SAMPLES_ZNUNU_MAXHTPTV_2}"
		    elif [[ "${CXAODTAG}" == "CxAODTag32" ]]; then
			USEZNUNUEXTENSION0LEPSAMPLES="true" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
			SAMPLES_ZNUNU_1="${SAMPLES_ZNUNU_MAXHTPTV_1} ${SAMPLES_ZNUNU_PTV_1}"
			SAMPLES_ZNUNU_2="${SAMPLES_ZNUNU_MAXHTPTV_2} ${SAMPLES_ZNUNU_PTV_AD_2}"
		    else
			echo "CXAODTAG=${CXAODTAG} not known. Choose CxAODTag31 or CxAODTag32! Abort!"
			exit 1
		    fi
		fi
		if [[ "${CHANNEL}" == "1L" ]]; then
		    USEZNUNUEXTENSION0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
		    SAMPLES_ZNUNU_1=""
		    SAMPLES_ZNUNU_2=""
		fi
		if [[ "${CHANNEL}" == "2L" ]]; then
		    USEZNUNUEXTENSION0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
		    SAMPLES_ZNUNU_1=""
		    SAMPLES_ZNUNU_2=""
		fi
	    elif [[ "${MCTYPE}" == "d" ]]; then
		LUMINOSITY="${LUMINOSITY_17}"
		ILUMICALCFILES="${ILUMICALCFILES_17}"
		CONFIGFILES="${CONFIGFILES_d}"
		# 1L PTW extension
		SAMPLESMCTYPE1+=" data17"
		# ZNUNU
		if [[ "${CHANNEL}" == "0L" ]]; then
		    USEZNUNUEXTENSION0LEPSAMPLES="true" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
		    SAMPLES_ZNUNU_1="${SAMPLES_ZNUNU_MAXHTPTV_1} ${SAMPLES_ZNUNU_PTV_1}"
		    SAMPLES_ZNUNU_2="${SAMPLES_ZNUNU_MAXHTPTV_2} ${SAMPLES_ZNUNU_PTV_AD_2}"
		fi
		if [[ "${CHANNEL}" == "1L" ]]; then
		    USEZNUNUEXTENSION0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
		    SAMPLES_ZNUNU_1=""
		    SAMPLES_ZNUNU_2=""
		fi
		if [[ "${CHANNEL}" == "2L" ]]; then
		    USEZNUNUEXTENSION0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
		    SAMPLES_ZNUNU_1=""
		    SAMPLES_ZNUNU_2=""
		fi
	    elif [[ "${MCTYPE}" == "e" ]]; then
		#
		if [[ "${CXAODTAG}" == "CxAODTag31" ]]; then
		    echo "Asking for period e in CxAODTag31, so we continue, skipping without breaking the code. This is to make it easier to submit a,d,e and 31-10,32-07 in one go."
		    continue
		fi
		#
		LUMINOSITY="${LUMINOSITY_18}"
		ILUMICALCFILES="${ILUMICALCFILES_18}"
		CONFIGFILES="${CONFIGFILES_e}"
		SAMPLESMCTYPE1+=" data18"

		SAMPLESMCTYPE2+=" "
		# ZNUNU
		if [[ "${CHANNEL}" == "0L" ]]; then
		    # since we use only PTV samples as default, there is no extension, so PTV sammples at full weight
		    USEZNUNUEXTENSION0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
		    SAMPLES_ZNUNU_1="${SAMPLES_ZNUNU_PTV_1}"
		    SAMPLES_ZNUNU_2="${SAMPLES_ZNUNU_PTV_E_2}"
		fi
		if [[ "${CHANNEL}" == "1L" ]]; then
		    USEZNUNUEXTENSION0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
		    SAMPLES_ZNUNU_1=""
		    SAMPLES_ZNUNU_2=""
		fi
		if [[ "${CHANNEL}" == "2L" ]]; then
		    USEZNUNUEXTENSION0LEPSAMPLES="false" # only does something for 0lep selection, and nothing for the others (1lep, 2lep)
		    SAMPLES_ZNUNU_1=""
		    SAMPLES_ZNUNU_2=""
		fi
		# 1L muon trigger
		if [[ "${CHANNEL}" == "1L" ]]; then
		    DO1LMUONTRIGGER="false" # only use muon trigger, if false it uses the MET trigger
		fi
	    else
		echo "MCTYPE=${MCTYPE} not known. Choose a, c or d. Will ABORT!!!"
		exit 1
	    fi
	    PERIOD=${MCTYPE}
	    echo "PERIOD=${PERIOD}"
	    echo "LUMINOSITY=${LUMINOSITY}"
	    echo "ILUMICALCFILES=${ILUMICALCFILES}"
	    echo "CONFIGFILES=${CONFIGFILES}"
	    echo "USEZNUNUEXTENSION0LEPSAMPLES=${USEZNUNUEXTENSION0LEPSAMPLES}"
	    echo "USETTBARPTWFILTERED1LEPSAMPLES=${USETTBARPTWFILTERED1LEPSAMPLES}"
	    # 
	    SAMPLESMCTYPE1+=" ${SAMPLES_ZNUNU_1}"
	    SAMPLESMCTYPE2+=" ${SAMPLES_ZNUNU_2}"
	    SAMPLES1="${SAMPLESMCTYPE1}"
	    SAMPLES2="${SAMPLESMCTYPE2}"
            # SAMPLES1
	    echo "BEFORE potential overwrite:"
	    echo "SAMPLES1=${SAMPLES1}"
	    if [[ ${SAMPLESFINAL1} == "none" ]] 
	    then
		echo "SAMPLESFINAL1 is not set, or it is set to an empty string, so we let SAMPLE1 as it is, without overwriting it with SAMPLESFINAL1."
	    else
		echo "SAMPLESFINAL1 is set and it is set to a non-empty string, so we overwrite SAMPLE1 with SAMPLESFINAL1."
		echo "SAMPLESFINAL1=${SAMPLESFINAL1}"
		SAMPLES1="${SAMPLESFINAL1}"
	    fi
	    echo "AFTER potential overwrite:"
	    echo "SAMPLES1=${SAMPLES1}"
            # SAMPLES2
	    echo "BEFORE potential overwrite:"
	    echo "SAMPLES2=${SAMPLES2}"
	    if [[ ${SAMPLESFINAL2} == "none" ]] 
	    then
		echo "SAMPLESFINAL2 is not set, or it is set to an empty string, so we let SAMPLE2 as it is, without overwriting it with SAMPLESFINAL2."
	    else
		echo "SAMPLESFINAL2 is set and it is set to a non-empty string, so we overwrite SAMPLE2 with SAMPLESFINAL2."
		echo "SAMPLESFINAL2=${SAMPLESFINAL2}"
		SAMPLES2="${SAMPLESFINAL2}"
	    fi
	    echo "AFTER potential overwrite:"
	    echo "SAMPLES2=${SAMPLES2}"
	    DATASETDIR="${INPUT_FOLDER}/${DERIVATION}_13TeV/CxAOD_${VTAG}_${MCTYPE}"
	    YIELDFILE="${DATASETDIR}/yields.13TeV_sorted.txt"
	    echo "DATASETDIR=${DATASETDIR}"
	    echo "YIELDFILE=${YIELDFILE}"
	    echo "For CHANNEL=VTAG=${VTAG} ${CHANNEL} MCTYPE=${MCTYPE}, start looping over TAGGING"
	    # loop over TAGGING (D, D1, D2, T2, D, T, H)
	    for TAGGING in `echo "${TAGGINGs}" | awk -v RS=, '{print}'`
	    do
		echo ""
		echo "*** VTAG=${VTAG} CHANNEL=${CHANNEL} MCTYPE=${MCTYPE} TAGGING=${TAGGING} ***"
		if [[ "${TAGGING}" == "D" ]]; then
		    # direct tagging
		    DOTRUTHTAGGING="false"
		    DOHYBRIDTRUTHTAGGING="false"
		    # samples
		    SAMPLES="${SAMPLES1} ${SAMPLES2}"
		elif [[ "${TAGGING}" == "D1" ]]; then
		    # direct tagging
		    DOTRUTHTAGGING="false"
		    DOHYBRIDTRUTHTAGGING="false"
		    # samples
		    SAMPLES="${SAMPLES1}"
		elif [[ "${TAGGING}" == "D2" ]]; then
		    # direct tagging
		    DOTRUTHTAGGING="false"
		    DOHYBRIDTRUTHTAGGING="false"
		    # samples
		    SAMPLES="${SAMPLES2}"
		elif [[ "${TAGGING}" == "T" ]]; then
                    # regular truth tagging
		    DOTRUTHTAGGING="true"
		    DOHYBRIDTRUTHTAGGING="false"
		    # samples
		    SAMPLES="${SAMPLES1} ${SAMPLES2}"
		elif [[ "${TAGGING}" == "T2" ]]; then
                    # regular truth tagging
		    DOTRUTHTAGGING="true"
		    DOHYBRIDTRUTHTAGGING="false"
		    # samples
		    SAMPLES="${SAMPLES2}"
		elif [[ "${TAGGING}" == "H" ]]; then
                    # hybrid truth tagging
		    DOTRUTHTAGGING="true"
		    DOHYBRIDTRUTHTAGGING="true"
		    # samples
		    SAMPLES="${SAMPLES1} ${SAMPLES2}"
		else
		    echo "TAGGING=${TAGGING} is not known. Choose D, D1, D2, T, T2, H. Will ABORT!!!"
		    exit 1
		fi
		echo "DOTRUTHTAGGING=${DOTRUTHTAGGING}"
		echo "DOHYBRIDTRUTHTAGGING=${DOHYBRIDTRUTHTAGGING}"
		echo "SAMPLES=${SAMPLES}"
		echo "For CHANNEL=VTAG=${VTAG} ${CHANNEL} MCTYPE=${MCTYPE} TAGGING=${TAGGING}, start looping over MODELTYPE"
		# loop over MODELTYPE (MVA, CUT)
		for MODELTYPE in `echo "${MODELTYPEs}" | awk -v RS=, '{print}'`
		do
		    echo ""
		    echo "*** VTAG=${VTAG} CHANNEL=${CHANNEL} MCTYPE=${MCTYPE} TAGGING=${TAGGING} MODELTYPE=${MODELTYPE} ***"
		    DESCRIPTION="${CHANNEL}_${VTAG}_${MCTYPE}_${MODELTYPE}_${TAGGING}"
		    if [[ ${EOSFOLDERUSERSTEM} == "none" ]]; then
			EOSFOLDERUSER="${EOSFOLDERUSERSTEM}"
		    else
			mkdir -p ${EOSFOLDERUSERSTEM}
			EOSFOLDERUSER="${EOSFOLDERUSERSTEM}/${DESCRIPTION}"
			echo "EOSFOLDERUSER=${EOSFOLDERUSER}"
		    fi
	            # create config file
		    echo "Create configs file:"
		    CONFIG_USE="${CONFIG_INITIAL_STEM}_${DESCRIPTION}"
		    if [[ ${ANALYSIS} == "VHcc" ]]; then
			CONFIG_USE+="_${ANALYSIS}"
		    fi
		    CONFIG_USE+=".cfg"
		    echo "Config file to use: CONFIG_USE=${CONFIG_USE}"
		    CONFIG="${WorkDir_DIR}/${CONFIG_USE}"
		    echo "Create config file with full path: CONFIG=${CONFIG}"
		    cp ${CONFIG_INITIAL} ${CONFIG}
	            # update the config file with sed using delimiter | instead of /
                    # because the some of the ones below use path that has already / in name

		    VARIABLEs="ANALYSIS CXAODTAG DEBUG NUMBEROFEVENTS ANASTRATEGY TRACKJETCONTAINER FATJETCONTAINER SUBJETCONTAINER TRACKLINKNAME DOCOMTAGGING FORCETRACKJETDRMATCHING REGULARJETCONTAINER METCONTAINER RSEVALUE ACCOUNTINGGROUP LEPCHANNEL MODELTYPE DOONLYINPUTS DOREDUCEDHISTOS DOSTOREBJETENERGYCORRHISTOS APPLYVZBBWEIGHT USETTBARMETFILTERED0LEPSAMPLES USETTBARPTWFILTERED1LEPSAMPLES DO1LMUONTRIGGER DO2LMETTRIGGER DOMETANDMUONTRIGGER DONEWREGIONS DODYWFJCUT USEZNUNUEXTENSION0LEPSAMPLES DOREMOVEDILEPOVERLAP DRIVER BQUEUE CONDORQUEUE BTAGGINGCDIFILE BTAGGINGCONFIGS DOTRUTHTAGGING DOHYBRIDTRUTHTAGGING GENERATESTXSSIGNALS REGULARJETCORRTYPE FATJETCORRTYPE RESOLVEDTAGSTRATEGY BOOSTEDTAGSTRATEGY WRITEHISTOGRAMS WRITEMVATREE WRITEASYTREE WRITEOSTREE DO_ICHEP NOMINALONLY USEQUANTILE SYSTEMATICSEXPERIMENTAL SYSTEMATICSMODELLING SYSTEMATICSCORRECTIONS SYSTEMATICSWEIGHT SYSTEMATICSTRIGGER SYSTEMATICSBDTS JOBSIZELIMITMB NRFILESPERJOB DATASETDIR YIELDFILE LUMINOSITY ILUMICALCFILES CONFIGFILES PERIOD SAMPLES DOMERGEJETBINS EOSFOLDERUSER DOPTVSPLITTING250GEV DO_KF KF_CONFIG DOFSRRECOVERY CONDOROSPREFERENCE DOLARGEMCEVENTWEIGHTSREMOVAL SPLITBOOSTEDINPUTSADDJETS DOLOWERPTZ2LCUT DONJETCUT0L1L EVTWEIGHTMODE EXCLUDEDBTAGEIGENVECTORS"

		    for VARIABLE in ${VARIABLEs}
		    do
		        # echo "VARIABLE=${VARIABLE}"
			COMMAND="echo ${VARIABLE}=\$${VARIABLE}"
		        # echo "COMMAND=${COMMAND}"
			eval ${COMMAND}
		        # sed -i -e 's|NRFILESPERJOB|'"${NRFILESPERJOB}"'|g' ${CONFIG}
			COMMAND="sed -i -e 's|${VARIABLE}|'\"\${${VARIABLE}}\"'|g' ${CONFIG}"
		        # echo "COMMAND=${COMMAND}"
			eval ${COMMAND}
		    done # done loop over variables
                    # copy the config file to the output_folder if switch is on
		    if [[ ${WRITEREADERCONFIG} == "true" ]]; then
			echo "cp ${CONFIG} ${OUTPUT_FOLDER}/";
			cp ${CONFIG} ${OUTPUT_FOLDER}/.
		    fi
	            # create the command and submit
		    OUTPUT_FOLDER_USE="${OUTPUT_FOLDER}/Reader_${CHANNEL}_${VTAG}_${MCTYPE}_${MODELTYPE}_${TAGGING}"
		    echo "OUTPUT_FOLDER_USE=${OUTPUT_FOLDER_USE}"
		    echo "CONFIG_USE=${CONFIG_USE}"
		    COMMAND="hsg5frameworkReadCxAOD ${OUTPUT_FOLDER_USE} ${CONFIG_USE}"
            if [[ ${WRITELOGOUTPUT} == "true" ]]; then
                COMMAND+=" | tee ${OUTPUT_FOLDER}/log_${CHANNEL}_${VTAG}_${MCTYPE}_${MODELTYPE}_${TAGGING}.out"
            fi
		    echo "COMMAND=${COMMAND}"
		    if [[ ${DO_EXECUTE} == "1" ]]; then
			eval ${COMMAND}
		    fi
		done # done loop over MODELTYPE
	    done # done loop over TAGGING
        done # done for loop over VTAG
    done # done for loop over MCTYPE
done # done for loop over CHANNEL

# #########################################################################################################
# Done all                                                                                          #######
# #########################################################################################################

echo ""
echo ""
echo "All finished well in submitReader!"
exit
