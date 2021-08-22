#!/usr/bin/env bash 
#if there is no parameter, it stops and it gives the instructions

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 == "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script forcing you to source. Don't execute me!" >&2
    exit 1
fi

# if there is no parameter, it stops and it gives the instructions
if [ $# -ne 9 ]; then
cat <<EOF
Usage: source $0 Channel  Sample Deriv   Stem        GridPrivilege VTag     Log     CxAODFramework                                              DoExecute
Usage: source $0 0L,1L,2L a,d,e  VHbb    mcdata      Higgs         32-07    none    $C 1
Usage: source $0 0L,1L,2L a,d,e  VHbb    mcdata      Higgs         32-07    long    $C 1
# Easiest is to first define C as the folder to your CxAODFramework, e.g.
C=/data06/abuzatu/code/CxAODFramework_branch_master_21.2.52_1
source $C/source/CxAODOperations_VHbb/scripts/getMaker.sh 0L,1L,2L a,d,e VHbb mcdata Higgs 32-02 $C 1
If separated by commma it means all will be done: Like all combinations are done of (0L,1L,2L)x(a,c,d)x(test,signal).
Deriv: currently supported only VHbb (used in CxAODTag32)
Usage: Stem can be test (some samples) or mcdata (all samples we run on). Each contain data, MC nominal, MC alternative, with MC both signal and background. 
Usage: Although we submit mcdata_nominal and mcdata_alternative separately to set 1 file per job for mcdata_nominal, we use them together in one go mcdata to retrieve from grid.
EOF
return
fi

CHANNELs=$1
SAMPLEs=$2
DERIV=$3
STEMs=$4
GRID_PRIVILEGE=$5
VTAG=$6
LOG_FILE=$7
CXAODFRAMEWORK=$8
DO_EXECUTE=$9

echo "Start getMaker.sh"
echo "CHANNELs=${CHANNELs}"
echo "SAMPLEs=${SAMPLEs}"
echo "DERIV=${DERIV}"
echo "STEMs=${STEMs}"
echo "GRID_PRIVILEGE=${GRID_PRIVILEGE}"
echo "VTAG=${VTAG}"
echo "LOG_FILE=${LOG_FILE}"
echo "CXAODFRAMEWORK=${CXAODFRAMEWORK}"
echo "DO_EXECUTE=${DO_EXECUTE}"

#
echo "We set the prefix as a function of the grid privileges."
if [[ ${GRID_PRIVILEGE} == "Higgs" ]]; then
    PREFIX="group.phys-higgs"
elif [[ ${GRID_PRIVILEGE} == "Exotics" ]]; then
    PREFIX="group.phys-exotics"
elif [[ ${GRID_PRIVILEGE} == "Hdbs" ]]; then
    PREFIX="group.phys-hdbs"
elif [[ ${GRID_PRIVILEGE} == "user" ]]; then
    PREFIX="user.${USER}"
else
    echo "GRID_PRIVILEGE=${GRID_PRIVILEGE} is not known. Choose Higgs, Exotics, Hdbs or user. Will ABORT!!!"
    return
fi
echo "PREFIX=${PREFIX}"

FOLDER=""

# set up CxAOD and grid
pushd ${CXAODFRAMEWORK}/run
source ../source/CxAODOperations_VHbb/scripts/setupLocal.sh
if [ -z $WorkDir_DIR ]; then
    echo "Environment variable WorkDir_DIR not set. Forgot to source the setup.sh?"
    return 1
fi
echo "Since we are here, CxAOD is well set up and WorkDir_DIR is defined as:"
echo "WorkDir_DIR=${WorkDir_DIR}"
pwd
source ../source/CxAODOperations_VHbb/scripts/setupGrid.sh ${GRID_PRIVILEGE} 1
popd


# loop over all the channels and periods
echo "We loop over the channels (e.g. 0L, 1L, 2L), for each set the appropriate derivation, then loop over the samples (e.g. a, c), and for each run the prepareCxAODProduction.sh script"
for CHANNEL in `echo "${CHANNELs}" | awk -v RS=, '{print}'`
do
    # 
    echo "CHANNEL=${CHANNEL}"
    if [[ ${CHANNEL} == "0L" ]]; then
	DERIVATION="HIGG5D1"
    elif [[ ${CHANNEL} == "1L" ]]; then
	DERIVATION="HIGG5D2"
    elif [[ ${CHANNEL} == "2L" ]]; then
	DERIVATION="HIGG2D4"
    else
	echo "CHANNEL=${CHANNEL} is not known. Choose 0L, 1L or 2L. Will SKIP it!!!"
	continue
    fi
    echo "DERIVATION=${DERIVATION}"
    # 
    for SAMPLE in `echo "${SAMPLEs}" | awk -v RS=, '{print}'`
    do
	echo "CHANNEL=${CHANNEL} SAMPLE=${SAMPLE}"
	for STEM in `echo "${STEMs}" | awk -v RS=, '{print}'`
	do
	    echo "CHANNEL=${CHANNEL} SAMPLE=${SAMPLE} STEM=${STEM}"
	    FOLDER+=" ${CHANNEL}_${SAMPLE}_${VTAG}"
	    if [[ ${LOG_FILE} == "none" ]]; then
		LOG_SUFFIX=""
	    elif [[ ${LOG_FILE} == "long" ]]; then
		LOG_SUFFIX=">& run_${CHANNEL}_${SAMPLE}_${VTAG}_${STEM}.log &"
	    else
		# use the name given by user
		LOG_SUFFIX=">& run_${LOG_FILE}.log &"
	    fi
	    COMMAND="source ${CXAODFRAMEWORK}/source/CxAODOperations_VHbb/scripts/prepareCxAODProduction.sh ${CXAODFRAMEWORK} prepare_${CHANNEL}_${SAMPLE}_${VTAG} ${CXAODFRAMEWORK}/source/CxAODOperations_VHbb/data/DxAOD/${DERIV}/list_sample_grid.13TeV_25ns.${STEM}_${SAMPLE}.${DERIVATION}.txt computeSampleList ${PREFIX} ${DERIVATION} ${VTAG} ${LOG_SUFFIX}"
	    echo "COMMAND=${COMMAND}"
	    if [[ ${DO_EXECUTE} == "1" ]]; then
		eval ${COMMAND}
	    fi
	done # done loop over STEM
    done # done loop over SAMPLE
done # done loop over CHANNEL


# in the main folder we copy the script that can copy from the grid for each of them
COMMAND="cp ${CXAODFRAMEWORK}/source/CxAODOperations_VHbb/scripts/runAllPrepareFolder.sh ."
echo "COMMAND=${COMMAND}"
if [[ ${DO_EXECUTE} == "1" ]]; then
    eval ${COMMAND}
fi
echo "FOLDER=${FOLDER}"
# use sed to replace FOLDERNAMES with the content of this folder
COMMAND="sed -i -e 's|FOLDERNAMES|${FOLDER}|g' runAllPrepareFolder.sh"
eval ${COMMAND}

# 
COMMAND="cp ${CXAODFRAMEWORK}/source/CxAODOperations_VHbb/scripts/killRucioPID.sh ."
echo "COMMAND=${COMMAND}"
if [[ ${DO_EXECUTE} == "1" ]]; then
    eval ${COMMAND}
fi


# 
COMMAND="cp ${CXAODFRAMEWORK}/source/CxAODOperations_VHbb/scripts/cleanPrepareFolder.sh ."
echo "COMMAND=${COMMAND}"
if [[ ${DO_EXECUTE} == "1" ]]; then
    eval ${COMMAND}
fi

# 
COMMAND="cp ${CXAODFRAMEWORK}/source/CxAODOperations_VHbb/scripts/operatePandaJobs.sh ."
echo "COMMAND=${COMMAND}"
if [[ ${DO_EXECUTE} == "1" ]]; then
    eval ${COMMAND}
fi

# 
COMMAND="cp ${CXAODFRAMEWORK}/source/CxAODOperations_VHbb/scripts/updateOutputSampleList.sh ."
echo "COMMAND=${COMMAND}"
if [[ ${DO_EXECUTE} == "1" ]]; then
    eval ${COMMAND}
fi

echo ""
echo ""
echo "All finished well for getMaker.sh."
return
