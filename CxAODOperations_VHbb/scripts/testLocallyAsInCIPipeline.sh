#!/usr/bin/env bash

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 == "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script ($0) forcing you to source. Don't execute me!" >&2
    exit 1
fi

# check the number of parameters, if not stop
if [ $# -ne 1 ]; then
cat <<EOF
Usage: $0 SAMPLE_INFO
Usage: $0 testLocallyAsInCIPipelineTasks.txt
Usage: $0 testLocallyAsInCIPipelineTasksReduced.txt
EOF
return 1
fi

SAMPLE_INFO=${1}
echo "SAMPLE_INFO=${SAMPLE_INFO}"

# set up the CxAOD if not set already
source ../source/CxAODOperations_VHbb/scripts/setupLocal.sh
if [ -z $WorkDir_DIR ]; then
    echo "Environment variable WorkDir_DIR not set. Forgot to source the setup.sh?"
    return 1
fi

DO_CREATE_FOR_SUBMITMAKER="0"
DO_CREATE_FOR_PILELINE="0"
DO_RUN_SUBMITMAKER="1"

STEM="none"
VTAG="none"
GRID="Higgs"
USE_PF="0"
USE_TCC="0"
LOG="none"
DO="1"

INPUT_FILE_NAME="${WorkDir_DIR}/data/CxAODOperations_VHbb/DxAOD/info/${SAMPLE_INFO}"
SAMPLE_STEM="/afs/cern.ch/work/v/vhbbframework/public/data/DxAOD"
OUTPUT_FILE_NAME_SUBMITMAKER="${WorkDir_DIR}/../../source/CxAODOperations_VHbb/data/DxAOD/info/forSubmitMakerTasks.txt"
OUTPUT_FILE_NAME_PIPELINE="${WorkDir_DIR}/../../source/CxAODOperations_VHbb/data/DxAOD/info/forCIPipelineTasks.txt"

echo "INPUT_FILE_NAME=${INPUT_FILE_NAME}"
echo "SAMPLE_STEM=${SAMPLE_STEM}"
echo "OUTPUT_FILE_NAME_SUBMITMAKER=${OUTPUT_FILE_NAME_SUBMITMAKER}"
echo "OUTPUT_FILE_NAME_PIPELINE=${OUTPUT_FILE_NAME_PIPELINE}"

if [[ ${DO_CREATE_FOR_SUBMITMAKER} == "1" ]]; then
    rm -f ${OUTPUT_FILE_NAME_SUBMITMAKER}
fi

if [[ ${DO_CREATE_FOR_PIPELINE} == "1" ]]; then
    rm -f ${OUTPUT_FILE_NAME_PIPELINE}
fi

ACTIONS=()

# start loop over CxAODFramework packages
COUNTER=0
while read line
do
    # skip coments and blank lines
    QUOTE_RE="^#"
    EMPTY_RE="^$"
    if [[ $line =~ $QUOTE_RE || $line =~ $EMPTY_RE ]] ; then
        continue
    fi
    # 
    COUNTER=$((COUNTER+1))
    echo "COUNTER=${COUNTER}"
    #
    SAMPLE_SHORT=$(echo "$line" | awk '{print $1}')
    CHANNEL=$(echo "$line" | awk '{print $2}')
    PERIOD=$(echo "$line" | awk '{print $3}')
    DERIV=$(echo "$line" | awk '{print $4}')
    NR_EVENTS=$(echo "$line" | awk '{print $5}')
    SAMPLE_LONG=$(echo "$line" | awk '{print $6}')
    # 
    echo ""
    echo "SAMPLE_SHORT=${SAMPLE_SHORT} CHANNEL=${CHANNEL} PERIOD=${PERIOD} DERIV=${DERIV} NR_EVENTS=${NR_EVENTS} SAMPLE_LONG=${SAMPLE_LONG}"
    LOCAL_SAMPLE="${SAMPLE_STEM}/${SAMPLE_LONG}"
    SUBMITDIR="${FOLDER_SUBMITDIR}/${SAMPLE_SHORT}"
    LOGFILE="${FOLDER_LOGFILE}/${SAMPLE_SHORT}.log"
    CONFIGFILE="${FOLDER_CONFIGFILE}/${SAMPLE_SHORT}.cfg"
    LOG_FILE="${SAMPLE_SHORT}"
    # 
    # create list of samples to run on in the pipeline
    if [[ ${DO_CREATE_FOR_SUBMITMAKER} == "1" ]]; then
	if [[ ${COUNTER} == "1" ]]; then
	    ACTIONS+=("  o Creating list of examples for submitMaker locally to copy/paste in submitMaker.sh.")
	    ACTIONS+=("      less ${OUTPUT_FILE_NAME_SUBMITMAKER}")
	fi
	LINES=()
	LINES+=("Usage: source $0 ${CHANNEL} ${PERIOD} ${DERIV} ${STEM} ${VTAG} ${GRID} ${USE_PF} ${USE_TCC} ${NR_EVENTS} ${SAMPLE_STEM}/${SAMPLE_LONG} ${LOG} ${DO}")
	for LINE in "${LINES[@]}"
	do
	    echo "${LINE}" >> ${OUTPUT_FILE_NAME_SUBMITMAKER}
	done
    fi
    # create list of samples to run on in the pipeline
    if [[ ${DO_CREATE_FOR_PILELINE} == "1" ]]; then
	if [[ ${COUNTER} == "1" ]]; then
	    ACTIONS+=("  o Creating list of action items for CI Pipeline to copy/paste in gitlab-ci.yml.")
	    ACTIONS+=("      less ${OUTPUT_FILE_NAME_PIPELINE}")
	fi
	LINES=()
	LINES+=("")
	LINES+=("${SAMPLE_SHORT}:")
	LINES+=("  variables:")
	LINES+=("    SAMPLE_SHORT: \"${SAMPLE_SHORT}\"")
	LINES+=("    CHANNEL: \"${CHANNEL}\"")
	LINES+=("    PERIOD: \"${PERIOD}\"")
	LINES+=("    DERIV: \"${DERIV}\"")
	LINES+=("    NR_EVENTS: \"${NR_EVENTS}\"")
	LINES+=("    SAMPLE_LONG: \"${SAMPLE_LONG}\"")
	LINES+=("  <<: *run_job")
	for LINE in "${LINES[@]}"
	do
	    echo "${LINE}" >> ${OUTPUT_FILE_NAME_PIPELINE}
	done
    fi
    # to submit the Maker
    if [[ ${DO_RUN_SUBMITMAKER} == "1" ]]; then
	if [[ ${COUNTER} == "1" ]]; then
	    ACTIONS+=("Submitting Maker locally.")
	fi
	COMMAND="source ../source/CxAODOperations_VHbb/scripts/submitMaker.sh ${CHANNEL} ${PERIOD} ${DERIV} ${STEM} ${VTAG} ${GRID} ${USE_PF} ${USE_TCC} ${NR_EVENTS} ${LOCAL_SAMPLE} ${LOG_FILE}_PF_${USE_PF} ${DO}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi
    # done for this sample
    echo "Done submitting for ${sampleShort}"
   # done all for current package
done < ${INPUT_FILE_NAME}
# done loop over all the packages

echo ""
echo ""
echo "Done in parallel for all jobs the following actions:"
for ACTION in "${ACTIONS[@]}"
do
    echo "${ACTION}"
done
echo "Done all!"

