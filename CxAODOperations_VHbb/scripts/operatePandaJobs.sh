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
if [ $# -ne 10 ]; then
cat <<EOF
Usage: $0 CHANNELS PERIODS VTAG        PANDAMON RETRY RESUBMIT KILL STEM   SUFFIX MY_USER
Usage: $0 0L,1L,2L a,d,e   32-07-3     1        0     0        0    mcdata 2      Adrian%20Buzatu
Usage: $0 0L,1L,2L a,d,e   32-07-3     1        1     0        0    mcdata 2      Adrian%20Buzatu
Usage: $0 0L,1L,2L a,d,e   32-07-3     1        1     1        0    mcdata 2      Adrian%20Buzatu
Usage: $0 1L       e       32-07-P-2   1        1     0        0    mcdata 2      Adrian%20Buzatu
Usage: $0 1L       e       32-07-P-2   1        0     1        0    mcdata 2      Adrian%20Buzatu
Usage: $0 1L       e       32-07-P-2   0        0     1        0    mcdata 2      Adrian%20Buzatu 
Usage: If RESUBMIT=1, then STEM and SUFFIX are used to create the DxAOD list to resubmit and the new output sample lists
MY_USER: Adrian%20Buzatu
MY_USER: Luca%20Ambroz 
MY_USER: Konie%20Al%20Khoury
MY_USER: Matthew%20Henry%20Klein
MY_USER: Mariia%20Didenko
MY_USER: Faig%20Ahmadov
MY_USER: Ricardo%20Filipe%20Coelho%20Barrue
MY_USER: Weitao%20Wang
MY_USER: Camilla%20Vittori%20cvittori@infn.it
EOF
exit 1
fi

# #########################################################################################################
# SETTINGS SET BY HAND                                                                            #########
# #########################################################################################################

CATEGORIES_RETRY="finished,failed,running,submitting"
CATEGORIES_RESUBMIT="broken,exhausted"
CATEGORIES_KILL="scouting,ready,prepared,running,exhausted" # all except broken, finished, failed which are already killed

echo "CATEGORIES_RETRY=${CATEGORIES_RETRY}"
echo "CATEGORIES_RESUBMIT=${CATEGORIES_RESUBMIT}"
echo "CATEGORIES_KILL=${CATEGORIES_KILL}"

# #########################################################################################################
# COMMAND LINE ARGUMENTS                                                                          #########
# #########################################################################################################

CHANNELS=$1
PERIODS=$2
VTAG=$3
DO_PANDAMON=$4
DO_RETRY=$5
DO_RESUBMIT=$6
DO_KILL=$7
STEM=$8
SUFFIX=$9
MY_USER=${10}

echo ""
echo ""
echo ""
echo "Start retryPandaJobs.sh"
echo "${CHANNELS}=CHANNELS"
echo "${PERIODS}=PERIODS"
echo "VTAG=${VTAG}"
echo "DO_PANDAMON=${DO_PANDAMON}"
echo "DO_RETRY=${DO_RETRY}"
echo "DO_RESUBMIT=${DO_RESUBMIT}"
echo "DO_KILL=${DO_KILL}"
echo "STEM=${STEM}"
echo "SUFFIX=${SUFFIX}"
echo "MY_USER=${MY_USER}"

# #########################################################################################################
# Run                                                                                                  ####
# https://gitlab.cern.ch/CxAODFramework/CxAODOperations_VHbb/blob/master/README_RunMaker.md
# #########################################################################################################

# loop over channel
for CHANNEL in `echo "${CHANNELS}" | awk -v RS=, '{print}'`
do
    echo "CHANNEL=${CHANNEL}"
    if [[ ${CHANNEL} == "0L" ]]; then
        DERIVATION="HIGG5D1"
    elif [[ ${CHANNEL} == "1L" ]]; then
        DERIVATION="HIGG5D2"
    elif [[ ${CHANNEL} == "2L" ]]; then
        DERIVATION="HIGG2D4"
    else
        echo "CHANNEL=${CHANNEL} is not known, so will ABORT!!!"
        exit -1
    fi
    # loop over period
    for PERIOD in `echo "${PERIODS}" | awk -v RS=, '{print}'`
    do
	echo "Start CHANNEL=${CHANNEL} PERIOD=${PERIOD}"
	# 
	if [[ ${DO_PANDAMON} == "1" ]]; then
	    echo "Start DO_PANDAMON CHANNEL=${CHANNEL}"
	    if [[ ${PERIOD} == "a" ]]; then
		pandamon *.*data15*.CAOD_${DERIVATION}.*.${VTAG}/ -d 40 --user ${MY_USER} > ${CHANNEL}_data15.log
		pandamon *.*data16*.CAOD_${DERIVATION}.*.${VTAG}/ -d 40 --user ${MY_USER} > ${CHANNEL}_data16.log
		pandamon *.*mc16*.CAOD_${DERIVATION}.*r9364*.${VTAG}/ -d 40 --user ${MY_USER} > ${CHANNEL}_mc16a.log
		cat ${CHANNEL}_data15.log ${CHANNEL}_data16.log ${CHANNEL}_mc16a.log > all_${CHANNEL}_${PERIOD}.log
	    elif [[ ${PERIOD} == "d" ]]; then
		pandamon *.*data17*.CAOD_${DERIVATION}.*.${VTAG}/ -d 40 --user ${MY_USER} > ${CHANNEL}_data17.log
		pandamon *.*mc16*.CAOD_${DERIVATION}.*r10201*.${VTAG}/ -d 40 --user ${MY_USER} > ${CHANNEL}_mc16d.log
		cat ${CHANNEL}_data17.log ${CHANNEL}_mc16d.log > all_${CHANNEL}_${PERIOD}.log
	    elif [[ ${PERIOD} == "e" ]]; then
		pandamon *.*data18*.CAOD_${DERIVATION}.*.${VTAG}/ -d 40 --user ${MY_USER} > ${CHANNEL}_data18.log
		pandamon *.*mc16*.CAOD_${DERIVATION}.*r10724*.${VTAG}/ -d 40 --user ${MY_USER} > ${CHANNEL}_mc16e.log
		cat ${CHANNEL}_data18.log ${CHANNEL}_mc16e.log > all_${CHANNEL}_${PERIOD}.log
	    else
		echo "PERIOD=${PERIOD} not known. Choose a, d, e. Will ABORT!!!"
		exit -1
	    fi
	    echo "End   DO_PANDAMON CHANNEL=${CHANNEL}"
	fi
	#
	if [[ ${DO_RETRY} == "1" ]]; then 
	    echo "Start DO_RETRY CHANNEL=${CHANNEL} PERIOD=${PERIOD}"
	    for CATEGORY in `echo "${CATEGORIES_RETRY}" | awk -v RS=, '{print}'`
	    do
		echo "Start DO_RETRY CHANNEL=${CHANNEL} PERIOD=${PERIOD} CATEGORY=${CATEGORY}"
		cat all_${CHANNEL}_${PERIOD}.log | awk -v cat=${CATEGORY} '$1 ~ cat {print $1 " " $2 " " $3 " " $4}'
		# cat all_${CHANNEL}_${PERIOD}.log | awk -v cat=${CATEGORY} '$1 ~ cat {print $2}' | panda-resub-taskid -s -g 3
		cat all_${CHANNEL}_${PERIOD}.log | awk -v cat=${CATEGORY} '$1 ~ cat {print $2}' | panda-resub-taskid
		echo "End   DO_RETRY CHANNEL=${CHANNEL} PERIOD=${PERIOD} CATEGORY=${CATEGORY}"
	    done
	    echo "End   DO_RETRY CHANNEL=${CHANNEL} PERIOD=${PERIOD}"
	fi
	#
	if [[ ${DO_RESUBMIT} == "1" ]]; then
	    echo "Start DO_RESUBMIT CHANNEL=${CHANNEL} PERIOD=${PERIOD}"
	    DXAOD_FILE_NAME="list_sample_grid.13TeV_25ns.${STEM}${SUFFIX}_${PERIOD}.${DERIVATION}.txt"
	    CXAOD_FILE_NAME="out_sample_${DXAOD_FILE_NAME}"
	    rm -f ${DXAOD_FILE_NAME}
	    rm -f ${CXAOD_FILE_NAME}
	    for CATEGORY in `echo "${CATEGORIES_RESUBMIT}" | awk -v RS=, '{print}'`
	    do
		echo "Start DO_RESUBMIT CHANNEL=${CHANNEL} PERIOD=${PERIOD} CATEGORY=${CATEGORY}"
		cat all_${CHANNEL}_${PERIOD}.log | awk -v cat=${CATEGORY} '$1 ~ cat {print $1 " " $2 " " $3 " " $4}'
		for taskname in `cat all_${CHANNEL}_${PERIOD}.log | awk -v cat=${CATEGORY} '$1 ~ cat {print $4}'`
		do 
		    # e.g. group.phys-higgs.data18_13TeV.00356205.CAOD_HIGG5D2.f956_m2004_p3640.32-07-3/
		    echo "taskname=${taskname}"
		    first="${taskname%.*}" # Removes from the last dot until the end and also the dot. Keeps what is before the last dot. 
		    echo "first=${first}"
		    # e.g. first="group.phys-higgs.data18_13TeV.00356205.CAOD_HIGG5D2.f956_m2004_p3640"
		    last="${taskname##*.}" # Removes until the last dot and also the dot. Keeps what is after the last dot.
		    echo "last=${last}"
		    # e.g. last="32-07-3/"
		    # removes the last /
		    lastshort="${last%/*}" # Removes from the last slash and also the slash. Keeps what is before the last bash.
		    echo "lastshort=${lastshort}"
		    # e.g. lastshort="32-07-3"
		    taskname_new="${first}.${lastshort}-${SUFFIX}"
		    echo "taskname_new=${taskname_new}"
		    # e.g. tasname_new="group.phys-higgs.data18_13TeV.00356205.CAOD_HIGG5D2.f956_m2004_p3640.32-07-3-2"
		    echo ${taskname_new} >> ${CXAOD_FILE_NAME}
		    pandamon -s IN ${taskname} | xargs echo >> ${DXAOD_FILE_NAME}
		done
		echo "End   DO_RESUBMIT CHANNEL=${CHANNEL} PERIOD=${PERIOD} CATEGORY=${CATEGORY}"
	    done
	    sort ${CXAOD_FILE_NAME} > ${CXAOD_FILE_NAME}_2
	    mv ${CXAOD_FILE_NAME}_2 ${CXAOD_FILE_NAME}
	    sort ${DXAOD_FILE_NAME} > ${DXAOD_FILE_NAME}_2
	    mv ${DXAOD_FILE_NAME}_2 ${DXAOD_FILE_NAME}
	    echo "End   DO_RESUBMIT CHANNEL=${CHANNEL} PERIOD=${PERIOD}"
	fi
	# 
	if [[ ${DO_KILL} == "1" ]]; then
	    echo "Start DO_KILL CHANNEL=${CHANNEL} PERIOD=${PERIOD}"
	    for CATEGORY in `echo "${CATEGORIES_KILL}" | awk -v RS=, '{print}'`
	    do
		echo "Start DO_KILL CHANNEL=${CHANNEL} PERIOD=${PERIOD} CATEGORY=${CATEGORY}"
		cat all_${CHANNEL}_${PERIOD}.log | awk -v cat=${CATEGORY} '$1 ~ cat {print $1 " " $2 " " $3 " " $4}'
		# for taskname in `cat all_${CHANNEL}_${PERIOD}.log | awk -v cat=${CATEGORY} '$1 ~ cat {print $4}'`
		cat all_${CHANNEL}_${PERIOD}.log | awk -v cat=${CATEGORY} '$1 ~ cat {print $2}'
		cat all_${CHANNEL}_${PERIOD}.log | awk -v cat=${CATEGORY} '$1 ~ cat {print $2}' | panda-kill-taskid
		echo "End   DO_KILL CHANNEL=${CHANNEL} PERIOD=${PERIOD} CATEGORY=${CATEGORY}"
	    done
	    echo "End   DO_KILL CHANNEL=${CHANNEL} PERIOD=${PERIOD}"
	fi
	# 
	echo "End   CHANNEL=${CHANNEL} PERIOD=${PERIOD}"
    done
    echo "End   CHANNEL=${CHANNEL}"
done

# #########################################################################################################
# Done all                                                                                          #######
# #########################################################################################################

echo ""
echo ""
echo "All finished well for operatePandaJobs.sh!"
exit
