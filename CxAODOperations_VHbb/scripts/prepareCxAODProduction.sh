#!/bin/bash
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 == "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script forcing you to source. Don't execute me!" >&2
    exit 1
fi  

#if there is no parameter, it stops and it gives the instructions
if [ $# -ne 7 ]; then
cat <<EOF
Usage: source $0 CxAODFolder WorkingFolder InputSampleList OutputSampleList  GridUser Derivation VTag         
Usage: if they exist already on the grid and you have the outfile and you want to download from the grid instead of from eos
Usage: source ../source/CxAODOperations_VHbb/scripts/prepareCxAODProduction.sh
Usage: 
Usage: if the outsample does not exist yet, and you run with Higgs group privileges
Usage: source ../source/CxAODOperations_VHbb/scripts/prepareCxAODProduction.sh /data06/abuzatu/code/CxAODFramework_branch_master_21.2.39_21 prepareCxAODProduction0LTest /data06/abuzatu/code/CxAODFramework_branch_master_21.2.39_21/source/CxAODOperations_VHbb/data/DxAOD/VHbb/list_sample_grid.13TeV_25ns.mcdata_a.HIGG5D1.txt computeSampleList group.phys-higgs HIGG5D1 CxAOD_Adrian
Usage: 
Usage: if you have your own grid username (and no privilege), replace group.phys-higgs with user.abuzatu
EOF
return
fi

SCRIPT_NAME=$0
CXAOD_FOLDER=$1
WORKING_FOLDER=$2
INPUT_SAMPLE_LIST=$3
OUTPUT_SAMPLE_LIST=$4
GRID_USER=$5
DERIVATION=$6
VTAG=$7

VERBOSE=1
DEBUG=0

echo "SCRIPT_NAME=${SCRIPT_NAME}"
echo "CXAOD_FOLDER=${CXAOD_FOLDER}"
echo "WORKING_FOLDER=${WORKING_FOLDER}"
echo "INPUT_SAMPLE_LIST=${INPUT_SAMPLE_LIST}"
echo "OUTPUT_SAMPLE_LIST=${OUTPUT_SAMPLE_LIST}"
echo "GRID_USER=${GRID_USER}"
echo "DERIVATION=${DERIVATION}"
echo "VTAG=${VTAG}"

# JOBVERSION="${DERIVATION}.${VTAG}"
# echo "JOBVERSION=${JOBVERSION}"

CXAOD_NUMBER="${VTAG:0:2}" # first two characters
echo "CXAOD_NUMBER=${CXAOD_NUMBER}"

STEM_LIST_SAMPLE="list_sample_grid.mc15c_13TeV_25ns"

mkdir -p ${WORKING_FOLDER}

# if the user wants to take just all from the grid, then use "all" for both input and output
# we loop over all of them and copy the combined list of input and output
if [[ ${INPUT_SAMPLE_LIST} == "all" ]]; then
    # the output must also be all
    if [[ ${OUTPUT_SAMPLE_LIST} != "all" ]]; then
	echo "If Input is all, the Output must also be all. We ABORT!!!"
	return -1
    fi

    ##################################################################################################
    ########## input files ###########################################################################
    ##################################################################################################

    # wc -l ${CXAOD_FOLDER}/CxAODOperations_VHbb/data/DxAOD/CxAOD${CXAOD_NUMBER}/${STEM_LIST_SAMPLE}*${DERIVATION}*.txt
    INPUT_SAMPLE_LIST="${STEM_LIST_SAMPLE}_${DERIVATION}_all.txt"
    cat ${CXAOD_FOLDER}/CxAODOperations_VHbb/data/DxAOD/CxAOD${CXAOD_NUMBER}/${STEM_LIST_SAMPLE}*${DERIVATION}*.txt > ${INPUT_SAMPLE_LIST}
    sort ${INPUT_SAMPLE_LIST} -o ${INPUT_SAMPLE_LIST}

    precedant_sample=""
    precedant_dsid=""
    while read sample; do
	# echo "sample=${sample}"
	dsid=`echo ${sample} | cut -d '.' -f2`
	# echo "precedant_dsid=${precedant_dsid} dsid=${dsid}"
	if [[ "${dsid}" == "${precedant_dsid}" ]]; then
	   echo "precedant_sample ${precedant_sample}"
	   echo "sample           ${sample}"
	fi
	precedant_sample="${sample}"
	precedant_dsid="${dsid}"
    done < ${INPUT_SAMPLE_LIST}

    # remove the ones that have the same DSID, but either another production/derivation tag, or AFII instead of FS
    INPUT_SAMPLE_LIST_2="${INPUT_SAMPLE_LIST}_2"
    rm -f ${INPUT_SAMPLE_LIST_2}

    while read sample; do
	# echo "sample=${sample}"
	echo "${sample}" >> ${INPUT_SAMPLE_LIST_2}
    done < ${INPUT_SAMPLE_LIST}

    mv ${INPUT_SAMPLE_LIST_2} ${WORKING_FOLDER}/${INPUT_SAMPLE_LIST}

    echo ""
    echo ""

    ##################################################################################################
    ########## output files ##########################################################################
    ##################################################################################################

    # wc -l ${CXAOD_FOLDER}/CxAODOperations_VHbb/data/CxAOD/CxAOD${CXAOD_NUMBER}/${STEM_LIST_SAMPLE}*${DERIVATION}*.txt
    OUTPUT_SAMPLE_LIST="out_sample_${STEM_LIST_SAMPLE}_${DERIVATION}_all.txt"
    cat ${CXAOD_FOLDER}/CxAODOperations_VHbb/data/CxAOD/CxAOD${CXAOD_NUMBER}/out_sample_${STEM_LIST_SAMPLE}*${DERIVATION}*.txt > ${OUTPUT_SAMPLE_LIST}
    sort ${OUTPUT_SAMPLE_LIST} -o ${OUTPUT_SAMPLE_LIST}
    precedant_sample=""
    precedant_dsid=""
    while read sample; do
	# echo "sample=${sample}"
	dsid=`echo ${sample} | cut -d '.' -f4`
	# echo "precedant_dsid=${precedant_dsid} dsid=${dsid}"
	if [[ "${dsid}" == "${precedant_dsid}" ]]; then
	   echo "precedant_sample ${precedant_sample}"
	   echo "sample           ${sample}"
	fi
	precedant_sample="${sample}"
	precedant_dsid="${dsid}"
    done < ${OUTPUT_SAMPLE_LIST}

    # remove the ones that have the same DSID, but either another production/derivation tag, or AFII instead of FS
    OUTPUT_SAMPLE_LIST_2="${OUTPUT_SAMPLE_LIST}_2"
    rm -f ${OUTPUT_SAMPLE_LIST_2}

    while read sample; do
	# echo "sample=${sample}"
	echo "${sample}" >> ${OUTPUT_SAMPLE_LIST_2}
    done < ${OUTPUT_SAMPLE_LIST}

    mv ${OUTPUT_SAMPLE_LIST_2} ${WORKING_FOLDER}/${OUTPUT_SAMPLE_LIST}

fi # end if the user gives the list that is to be downloaded from the grid

# if the user gives an input list, and we want to create the output for it, or the user gives an output list
if [[ ${OUTPUT_SAMPLE_LIST} == "computeSampleList" ]]; then
    echo "Compute the output sample"
    # create output sample list (out_sample_*.list) # ADRIAN 
    COMMAND="${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/form_HSG5_out.py ${GRID_USER} ${VTAG} ${INPUT_SAMPLE_LIST} ${WORKING_FOLDER} ${DEBUG}"
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}
elif [[ ${OUTPUT_SAMPLE_LIST} == "all" ]]; then
    echo "all, do nothing"
else
    echo "Use the output sample given to us"
    cp ${OUTPUT_SAMPLE_LIST} ${WORKING_FOLDER}
fi 

# create AMI event count (dslist_NevtDxAOD.txt)
COMMAND="${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/getAMIInfo.py ${INPUT_SAMPLE_LIST} ${WORKING_FOLDER} ${VERBOSE} ${DEBUG}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}

# copy script to download from the grid via rucio
COMMAND="cp ${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/rucio_get_jobs.py ${WORKING_FOLDER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}

# copy script that computes the sum of weights (both to compare with Ami and to produce the one to use)
COMMAND="cp ${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/count_Nentry_SumOfWeight.py ${WORKING_FOLDER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}
# once the files are downloaded locally, from that folder run
# ./count_Nentry_SumOfWeight.py 1 0 
# this produces a file called yields.13TeV_DxAOD_sorted.txt
# then you need to compare this file (yields.13TeV_DxAOD_sorted.txt) with that produced by AMI (dslist_NevtDxAOD.txt)
# you need the file 
COMMAND="cp ${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/checkYields.py ${WORKING_FOLDER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}
# which you run with
# ./checkYields.py
# it produces a file called dslist_NevtDxAOD_yield.txt and tell you if there are samples not yet downloaded that are in the initial list
# for the samples already here it compares the yields and you should have a ratio of 1 when all the files for that sample were copied

# to prepare the output sample list from the already downloaded CxAOD (and then compare and/or copy with/to the one from CxAODOperations_VHbb/data/CxAOD)
COMMAND="cp ${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/createOutputLisFromAlreadyDownloaded.sh ${WORKING_FOLDER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}

# when the agreement is full, you can copy the scripts to eos
COMMAND="cp ${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/copy_CxAODs_to_eos.py ${WORKING_FOLDER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}

# to copy to eos, you need the file of eos folder name for each DSID
COMMAND="cp ${CXAOD_FOLDER}/source/CxAODOperations_VHbb/data/DxAOD/info/sample_info.txt ${WORKING_FOLDER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}

# to replicate to grid
COMMAND="cp ${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/replicateToGrid.py ${WORKING_FOLDER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}

# to run repeatedly rucio_get_jobs.py
COMMAND="cp ${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/runRepeatedlyRucioGetJobs.sh ${WORKING_FOLDER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}

# to retry automatically jobs in Panda
COMMAND="cp ${CXAOD_FOLDER}/source/CxAODOperations_VHbb/scripts/operatePandaJobs.sh ${WORKING_FOLDER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}

echo ""
echo ""
echo "All finished well for prepareCxAODProduction.sh."
return
