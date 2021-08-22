#!/usr/bin/env bash
# if there is no parameter, it stops and it gives the instructions
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group 

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 == "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script ($0) forcing you to source. Don't execute me!" >&2
    exit 1
fi

# if there is no parameter, it stops and it gives the instructions
if [ $# != 12 ]; then
cat <<EOF
Usage: source $0 Channels MCTypes Deriv     Stem               Vtag          Grid    usePF useTCC NrEvent LocalSample Log Do
Usage:
Usage: Running all samples from a sample list in one go, on the grid
Usage: if Log argument is none, then each of the 9 (0L,1L,2L)x(a,d,e) is done one after the other
Usage: for mcdata it can take 12 hours or more; one way is to open 9 terminals and do in parallel
Usage: source $0 0L,1L,2L a,d,e   VHbb      test               32-09         Higgs   0   0      -1      none        none 1
Usage: source $0 0L,1L,2L a,d,e   VHbb      mcdata             32-09         Higgs   0   0      -1      none        none 1
Usage: but if you are on a machine with more cores, you can submit in parallel with one comman directly
Usage: replace the Log of none with the Log of long
Usage: source $0 0L,1L,2L a,d,e   VHbb      test               32-09         Higgs   0   0      -1      none        long 1
Usage: source $0 0L,1L,2L a,d,e   VHbb      mcdata             32-09         Higgs   0   0      -1      none        long 1

Usage:
Usage: running one one local sample at a time e.g.
Usage: source -bash 0L a VHbb none none Higgs 0 0 1000 /afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data16_13TeV.00311321.physics_Main.deriv.DAOD_HIGG5D1.r9264_p3083_p3640 none 1
For running over more test samples on afs, see:
scripts/runTestFiles.sh

Usage: 
Usage: More explanations
If separated by comma it means all will be done: Like all combinations are done of (0L,1L,2L)x(a,c,d)x(test,signal).
Deriv: choose between VHbb (used in CxAOD30 and CxAOD31) and monoHbb (asked for by mono-Hbb)
Usage: Stem can be test (some samples) or mcdata (all samples we run on). Each contain data, MC nominal, MC alternative, with MC both signal and background. 
EOF
return
fi

WRITELOG="1"

CHANNELs=$1
MCTYPEs=$2
DERIV=$3
STEM=$4
VTAG=$5
GRID_PRIVILEGE=$6
USE_PF=$7
USE_TCC=$8
NR_EVENTS=$9
LOCAL_SAMPLE=${10}
LOG_FILE=${11}
DO_EXECUTE=${12}

echo "Start submitToGrid.sh"
echo "CHANNELs=${CHANNELs}"
echo "MCTYPEs=${MCTYPEs}"
echo "DERIV=${DERIV}"
echo "STEM=${STEM}"
echo "VTAG=${VTAG}"
echo "GRID_PRIVILEGE=${GRID_PRIVILEGE}"
echo "USE_PF=${USE_PF}"
echo "USE_TCC=${USE_TCC}"
echo "NR_EVENTS=${NR_EVENTS}"
echo "DO_EXECUTE=${DO_EXECUTE}"
echo "LOG_FILE=${LOG_FILE}"
echo "LOCAL_SAMPLE=${LOCAL_SAMPLE}"

LOCAL_SAMPLE_BASENAME=$(basename ${LOCAL_SAMPLE})
echo "LOCAL_SAMPLE_BASENAME=${LOCAL_SAMPLE_BASENAME}"

source ../source/CxAODOperations_VHbb/scripts/setupLocal.sh
if [ -z $WorkDir_DIR ]; then
    echo "Environment variable WorkDir_DIR not set. Forgot to source the setup.sh?"
    return 1
fi
echo "Since we are here, CxAOD is well set up and WorkDir_DIR is defined as:"
echo "WorkDir_DIR=${WorkDir_DIR}"

DEBUG="false"
NUMBEROFEVENTS="${NR_EVENTS}" # "-1"
SUBMIT="true"
USEDSITE="all"
# USEDSITE="ANALY_TAIWAN_SL6"
EXCLUDEDSITE="none"
# EXCLUDEDSITE="ANALY_TRIUMF"
# EXCLUDEDSITE="ANALY_ARNES,ANALY_LUNARC"
DESTSE="none"
# DESTSE="JINR-LCG2_LOCALGROUPDISK"
USENEWCODE="false"
ALLOWTASKDUPLICATION="false"
# for mcdata_nominal (MC nominal with systematics)
# NRGBPERJOB="-1"
# NRFILESPERJOB="1"
# for mcdata_alternative (data and MC alternative)
NRGBPERJOB="10" # 1000 will set to MAX; use integer numbers, or they will be rounded
NRFILESPERJOB="-1" #
# note that keeping to false also for data and MC alternative is fine
# as AnalysisBase.cxx is smart enough to reset it for us to true
NOMINALONLY="false"
GRLFILES_1516="GoodRunsLists/data15_13TeV/20170619/physics_25ns_21.0.19.xml GoodRunsLists/data16_13TeV/20180129/physics_25ns_21.0.19.xml"
GRLFILES_17="GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.xml"
GRLFILES_18="GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.xml"
GRLFILES="${GRLFILES_1516} ${GRLFILES_17} ${GRLFILES_18}"
ILUMICALCFILES_1516="GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root"
ILUMICALCFILES_17="GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root"
ILUMICALCFILES_18="GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root"
CONFIGFILES_a="\$\{WorkDir_DIR\}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16a.root" # with averageMu
CONFIGFILES_c="\$\{WorkDir_DIR\}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16c.root" # with averageMu
CONFIGFILES_d="\$\{WorkDir_DIR\}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16d.root GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root" # with actualMu
# CONFIGFILES_d="\$\{WorkDir_DIR\}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16d.root" # with averageMu - not recommended
CONFIGFILES_e="\$\{WorkDir_DIR\}/data/CxAODOperations_VHbb/PRW/PRW.VHbb.mc16e.root GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.actualMu.OflLumi-13TeV-010.root" # with actualMu
STORETRUTHJET4VECTOR="false"

echo "DEBUG=${DEBUG}"
echo "NUMBEROFEVENTS=${NUMBEROFEVENTS}"
echo "SUBMIT=${SUBMIT}"
echo "USEDSITE=${USEDSITE}"
echo "EXCLUDEDSITE=${EXCLUDEDSITE}"
echo "DESTSE=${DESTSE}"
echo "USENEWCODE=${USENEWCODE}"
echo "ALLOWTASKDUPLICATION=${ALLOWTASKDUPLICATION}"
echo "NRGBPERJOB=${NRGBPERJOB}"
echo "NRFILESPERJOB=${NRFILESPERJOB}"
echo "NOMINALONLY=${NOMINALONLY}"
echo "GRLFILES_1516=${GRLFILES_1516}"
echo "GRLFILES_17=${GRLFILES_17}"
echo "GRLFILES_18=${GRLFILES_18}"
echo "GRLFILES=${GRLFILES}"
echo "ILUMICALCFILES_1516=${ILUMICALCFILES_1516}"
echo "ILUMICALCFILES_17=${ILUMICALCFILES_17}"
echo "ILUMICALCFILES_18=${ILUMICALCFILES_18}"
echo "CONFIGFILES_a=${CONFIGFILES_a}"
echo "CONFIGFILES_c=${CONFIGFILES_c}"
echo "CONFIGFILES_d=${CONFIGFILES_d}"
echo "CONFIGFILES_e=${CONFIGFILES_e}"

CONFIG_INITAL_STEM="data/CxAODMaker_VHbb/framework-run-automatic"
CONFIG_INITIAL="${WorkDir_DIR}/${CONFIG_INITAL_STEM}.cfg"
echo "CONFIG_INITIAL=${CONFIG_INITIAL}"

if [[ "${LOCAL_SAMPLE}" == "none" ]]; then
    echo "LOCAL_SAMPLE==none, so you want to run on grid. Setting up grid permissions"
    # set grid
    if [ ${GRID_PRIVILEGE} == "Higgs" ] || [ ${GRID_PRIVILEGE} == "Exotics" ] || [ ${GRID_PRIVILEGE} == "Hdbs" ] || [ ${GRID_PRIVILEGE} == "user" ]; then
	COMMAND="source ../source/CxAODOperations_VHbb/scripts/setupGrid.sh ${GRID_PRIVILEGE} ${DO_EXECUTE}"
	if [ ${GRID_PRIVILEGE} == "Higgs" ]; then
	    EXTRA="--higgs"
	elif [ ${GRID_PRIVILEGE} == "Exotics" ]; then
	    EXTRA="--exotics"
	elif [ ${GRID_PRIVILEGE} == "Hdbs" ]; then
	    EXTRA="--hdbs"
	else
	    EXTRA=""
	fi # end if GRID_PRIVILEGE
    else
	echo "GRID_PRIVILEGE=${GRID_PRIVILEGE} is not known. Choose Higgs, Exotics, Hdbs, or user. Will ABORT!!!"
	return
    fi # end if GRID_PRIVILEGE OR
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}
else
    echo "Running on local sample, so not setting up the grid."
    #cd run
fi # end if LOCAL_SAMPLE

# return

# start doing the actual work by looping over channels and mcTypes
echo "We set up CxAODFramework and GRID_PRIVILEGE=${GRID_PRIVILEGE}. We are already in the run folder, so we proceed to submit to the grid."

# regular jet choice
if [ ${USE_PF} == "1" ]; then
    # ParticleFlow (PF)
    REGULARJETCONTAINER="AntiKt4EMPFlowJets"
    REGULARJETALGONAME="AntiKt4EMPFlow"
    METCONTAINER="MET_Reference_AntiKt4EMPFlow"
else
    # EMTopo
    REGULARJETCONTAINER="AntiKt4EMTopoJets"
    REGULARJETALGONAME="AntiKt4EMTopo"
    METCONTAINER="MET_Reference_AntiKt4EMTopo"
fi
echo "REGULARJETCONTAINER=${REGULARJETCONTAINER}"
echo "REGULARJETALGONAME=${REGULARJETALGONAME}"
echo "METCONTAINER=${METCONTAINER}"

# fat jet choice
FATJETVARIATIONS=""
if [ ${USE_TCC} == "1" ]; then
    # TrackCaloCluster (TCC)
    FATJETCONTAINER="AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20Jets"
    FATJETUNCBASE="rel21/Summer2019/"
    FATJETUNCPREFIXES="FATJET_All_"
    FATJETUNCCONFIGS="R10_Scale_TCC_all"
    FATJETVARIATIONS="FATJET_JER FATJET_All_JET_Rtrk_Baseline_pT FATJET_All_JET_Rtrk_Tracking_pT FATJET_All_JET_Rtrk_TotalStat_pT FATJET_All_JET_Rtrk_Modelling_pT FATJET_All_JET_Rtrk_Closure_pT FATJET_All_JET_Rtrk_Baseline_m FATJET_All_JET_Rtrk_Tracking_m FATJET_All_JET_Rtrk_TotalStat_m FATJET_All_JET_Rtrk_Modelling_m FATJET_All_JET_Rtrk_Closure_m"
else
    # LCTopo
    FATJETCONTAINER="AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets"
    FATJETUNCBASE="rel21/Summer2019/"
    FATJETUNCPREFIXES="FATJET_All_ FATJET_CR_ FATJET_GR_"
    FATJETUNCCONFIGS="R10_AllNuisanceParameters R10_CategoryReduction R10_GlobalReduction"
    FATJETVARIATIONS=" FATJET_CR_JET_EtaIntercalibration_Modelling FATJET_CR_JET_EtaIntercalibration_R10_TotalStat FATJET_CR_JET_Flavor_Composition FATJET_CR_JET_Flavor_Response FATJET_CR_JET_EffectiveNP_R10_Statistical1 FATJET_CR_JET_EffectiveNP_R10_Statistical2 FATJET_CR_JET_EffectiveNP_R10_Statistical3 FATJET_CR_JET_EffectiveNP_R10_Statistical4 FATJET_CR_JET_EffectiveNP_R10_Statistical5 FATJET_CR_JET_EffectiveNP_R10_Statistical6 FATJET_CR_JET_EffectiveNP_R10_Modelling1 FATJET_CR_JET_EffectiveNP_R10_Modelling2 FATJET_CR_JET_EffectiveNP_R10_Modelling3 FATJET_CR_JET_EffectiveNP_R10_Modelling4 FATJET_CR_JET_EffectiveNP_R10_Detector1 FATJET_CR_JET_EffectiveNP_R10_Detector2 FATJET_CR_JET_EffectiveNP_R10_Mixed1 FATJET_CR_JET_EffectiveNP_R10_Mixed2 FATJET_CR_JET_EffectiveNP_R10_Mixed3 FATJET_CR_JET_EffectiveNP_R10_Mixed4 FATJET_CR_JET_SingleParticle_HighPt FATJET_CR_JET_LargeR_TopologyUncertainty_V FATJET_CR_JET_LargeR_TopologyUncertainty_top FATJET_CR_JET_EtaIntercalibration_NonClosure_2018data FATJET_CR_JET_CombMass_Baseline FATJET_CR_JET_CombMass_Modelling FATJET_CR_JET_CombMass_Tracking1 FATJET_CR_JET_CombMass_Tracking2 FATJET_CR_JET_CombMass_Tracking3 FATJET_CR_JET_CombMass_TotalStat FATJET_JER FATJET_JMR FATJET_SubR FATJET_Medium_JET_Rtrk_Baseline_Sub FATJET_Medium_JET_Rtrk_Modelling_Sub FATJET_Medium_JET_Rtrk_TotalStat_Sub FATJET_Medium_JET_Rtrk_Tracking_Sub"
fi
echo "FATJETCONTAINER=${FATJETCONTAINER}"
echo "FATJETUNCBASE=${FATJETUNCBASE}"
echo "FATJETUNCPREFIXES=${FATJETUNCPREFIXES}"
echo "FATJETUNCCONFIGS=${FATJETUNCCONFIGS}"
echo "FATJETVARIATIONS=${FATJETVARIATIONS}"

# Lepton Isolation
if [ ${DERIV} == "DBL" -o ${DERIV} == "DBLPatch" ]; then
    # VHLoose Lepton Iso for DBL
    VVSEMILEPTONICELECTRONISO=true
    VVSEMILEPTONICMUONISO=true
    STORETRUTHJET4VECTOR=true
else
    # VHLoose Lepton Iso for VHbb
    VVSEMILEPTONICELECTRONISO=false
    VVSEMILEPTONICMUONISO=false
fi

echo "VVSEMILEPTONICELECTRONISO=${VVSEMILEPTONICELECTRONISO}"
echo "VVSEMILEPTONICMUONISO=${VVSEMILEPTONICMUONISO}"
echo "STORETRUTHJET4VECTOR=${STORETRUTHJET4VECTOR}"

# TruthWZ
TRUTHWZJETCONTAINER="AntiKt4TruthWZJets"
echo "TRUTHWZJETCONTAINER=${TRUTHWZJETCONTAINER}"

# if to store variables used to do quark-gluon tagging
SAVEQGVARIABLES="true"
echo "SAVEQGVARIABLES=${SAVEQGVARIABLES}"

#lower pTZ cut for VHbb
if [ ${DERIV} == "DBL" -o ${DERIV} == "DBLPatch" ]; then
    DOLOWERPTZ2LCUT=false
    DONJETCUT0L1L=false
else
    DOLOWERPTZ2LCUT=true
    DONJETCUT0L1L=true
fi

echo "DOLOWERPTZ2LCUT=${DOLOWERPTZ2LCUT}"
echo "DONJETCUT0L1L=${DONJETCUT0L1L}"

# loop over channels
echo "Start looping over CHANNEL:"
for CHANNEL in `echo "${CHANNELs}" | awk -v RS=, '{print}'`
do
    echo "CHANNEL=${CHANNEL}"
    if [[ "${CHANNEL}" == "0L" ]]; then
	LEPCHANNEL="0lep"
	DERIVATION="HIGG5D1"
	METTRIGGEREMULATION="true"
	COMPUTECODETTBARDECAY="true"
	DOXBBTAG="true"
	DOCLOSEBYISOCORR="false"
	QGTAGGER="false"
    elif [[ "${CHANNEL}" == "1L" ]]; then
	LEPCHANNEL="1lep"
	DERIVATION="HIGG5D2"
	METTRIGGEREMULATION="true"
	COMPUTECODETTBARDECAY="true"
	DOXBBTAG="true"
	DOCLOSEBYISOCORR="false"
	QGTAGGER="false"
    elif [[ "${CHANNEL}" == "2L" ]]; then
	LEPCHANNEL="2lep"
	DERIVATION="HIGG2D4"
	METTRIGGEREMULATION="false"
	COMPUTECODETTBARDECAY="true"
	DOXBBTAG="true"
	DOCLOSEBYISOCORR="false"
	QGTAGGER="false"
    else
	echo "CHANNEL=${CHANNEL} not known. Choose 0L, 1L or 2L. Will ABORT!!!"
	return
    fi # end if CHANNEL

    # store the boson-tagging results to CxAOD?
    DOBOSONTAG="true"

    echo "LEPCHANNEL=${LEPCHANNEL}"
    echo "DERIVATION=${DERIVATION}"
    echo "DOXBBTAG=${DOXBBTAG}"
    echo "DOBOSONTAG=${DOBOSONTAG}"
    echo "DOCLOSEBYISOCORR=${DOCLOSEBYISOCORR}"
    echo "METTRIGGEREMULATION=${METTRIGGEREMULATION}"
    echo "Start looping over MCTYPE:"
    for MCTYPE in `echo "${MCTYPEs}" | awk -v RS=, '{print}'`
    do
	echo "CHANNEL=${CHANNEL} MCTYPE=${MCTYPE}"
	if [[ "${LOCAL_SAMPLE}" != "none" ]]; then
	    SAMPLES=${LOCAL_SAMPLE}
	else
	   if [[ "${STEM}" == "none" ]]; then
	       echo "Since LOCAL_SAMPLE==none, STEM must not be none, but it is none. Will ABORT!!!"
	       return
	   fi
	   if [[ "${VTAG}" == "none" ]]; then
	       echo "Since LOCAL_SAMPLE==none, VTAG must not be none, but it is none. Will ABORT!!!"
	       return
	   fi
	   SAMPLES="../source/CxAODOperations_VHbb/data/DxAOD/${DERIV}/list_sample_grid.13TeV_25ns.${STEM}_${MCTYPE}.${DERIVATION}.txt"
	fi
	echo "SAMPLE=${SAMPLE}"
	if [[ "${MCTYPE}" == "a" ]]; then
	    ILUMICALCFILES="${ILUMICALCFILES_1516}"
	    CONFIGFILES="${CONFIGFILES_a}"
	elif [[ "${MCTYPE}" == "c" ]]; then
	    ILUMICALCFILES="${ILUMICALCFILES_17}"
	    CONFIGFILES="${CONFIGFILES_c}"
	elif [[ "${MCTYPE}" == "d" ]]; then
	    ILUMICALCFILES="${ILUMICALCFILES_17}"
	    CONFIGFILES="${CONFIGFILES_d}"
	elif [[ "${MCTYPE}" == "e" ]]; then
	    ILUMICALCFILES="${ILUMICALCFILES_18}"
	    CONFIGFILES="${CONFIGFILES_e}"
	else
	    echo "MCTYPE=${MCTYPE} not known. Choose a, c, d or e. Will ABORT!!!"
	    return
	fi
	echo "ILUMICALCFILES=${ILUMICALCFILES}"
	echo "CONFIGFILES=${CONFIGFILES}"
	#
	RUN_INFO="${CHANNEL}_${MCTYPE}_${DERIV}_${STEM}_PF_${USE_PF}_TCC_${USE_TCC}_${LOCAL_SAMPLE_BASENAME}_${VTAG}"
	echo "RUN_INFO=${RUN_INFO}"
	if [[ ${LOG_FILE} == "none" ]]; then
	    PREFIX=""
	    LOG_SUFFIX=""
	    OUTPUT_FOLDER="Maker_${RUN_INFO}"    
	elif [[ ${LOG_FILE} == "long" ]]; then
	    PREFIX="nohup "
	    LOG_SUFFIX=">& logs/Maker_${RUN_INFO}.log &"
	    OUTPUT_FOLDER="Maker_${RUN_INFO}"
	else
            # use the name given by user, so that for test as in pipeline to use the short name
	    PREFIX="nohup "
	    LOG_SUFFIX=">& logs/Maker_${LOG_FILE}.log &"
	    OUTPUT_FOLDER="Maker_${LOG_FILE}"
	fi
	echo "LOG_SUFFIX=${LOG_SUFFIX}"
	echo "OUTPUT_FOLDER=${OUTPUT_FOLDER}"
        # create config file
	echo "Create configs file:"
	CONFIG_USE="${CONFIG_INITAL_STEM}_${RUN_INFO}.cfg"
	echo "Config file to use: CONFIG_USE=${CONFIG_USE}"
	CONFIG="${WorkDir_DIR}/${CONFIG_USE}"
	echo "Create config file with full path: CONFIG=${CONFIG}"
	cp ${CONFIG_INITIAL} ${CONFIG}
	# update the config file with sed using delimiter | instead of /
        # because the some of the ones below use path that has already / in name
	VARIABLEs="NUMBEROFEVENTS SAMPLES LEPCHANNEL GRLFILES ILUMICALCFILES CONFIGFILES DOXBBTAG DOBOSONTAG DOCLOSEBYISOCORR REGULARJETCONTAINER REGULARJETALGONAME METCONTAINER FATJETCONTAINER FATJETUNCBASE FATJETUNCPREFIXES FATJETUNCCONFIGS TRUTHWZJETCONTAINER METTRIGGEREMULATION DEBUG NOMINALONLY VTAG SUBMIT USEDSITE EXCLUDEDSITE DESTSE USENEWCODE ALLOWTASKDUPLICATION NRGBPERJOB NRFILESPERJOB COMPUTECODETTBARDECAY QGTAGGER SAVEQGVARIABLES VVSEMILEPTONICELECTRONISO VVSEMILEPTONICMUONISO STORETRUTHJET4VECTOR FATJETVARIATIONS DOLOWERPTZ2LCUT DONJETCUT0L1L" 
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
	# submit
	COMMAND="${PREFIX} hsg5framework ${EXTRA} --submitDir ${OUTPUT_FOLDER} --config ${CONFIG_USE} ${LOG_SUFFIX}"
	echo "COMMAND=${COMMAND}"
	if [[ ${DO_EXECUTE} == "1" ]]; then
	    eval ${COMMAND}
	    cp ${CONFIG} configs/.
	fi
    done
done

echo "All finished well for submitMaker.sh."
return
