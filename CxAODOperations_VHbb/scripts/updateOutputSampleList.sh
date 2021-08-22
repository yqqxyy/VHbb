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
    # set -eu
    true
fi

# if there is no parameter, it stops and it gives the instructions
if [ $# -ne 2 ]; then
cat <<EOF
Usage: $0 INITIAL                                                     REPLACEMENT
Usage: $0 out_sample_list_sample_grid.13TeV_25ns.mcdata_d.HIGG5D2.txt out_sample_list_sample_grid.13TeV_25ns.mcdata2_d.HIGG5D2.txt
EOF
exit 1
fi

# #########################################################################################################
# COMMAND LINE ARGUMENTS                                                                          #########
# #########################################################################################################

INITIAL=$1
REPLACEMENT=$2

echo "Start updateOutputSampleList.sh"
echo "INITIAL=${INITIAL}"
echo "REPLACEMENT=${REPLACEMENT}"

DEBUG="0"

# #########################################################################################################
# Create a backup and update the initial file                                                     #########
# #########################################################################################################


# create a backup with a time stamp
TIME_STAMP="$(date +%Y%m%d_%H%M%S)"
echo "Creating a backup of ${INITIAL} with the TIME_STAMP=${TIME_STAMP}"
cp ${INITIAL} bk_${TIME_STAMP}_${INITIAL}

# loop over all the lines of ${REPLACEMENT}
while read line
do
    # skip coments and blank lines
    QUOTE_RE="^#"
    EMPTY_RE="^$"
    if [[ $line =~ $QUOTE_RE || $line =~ $EMPTY_RE ]] ; then
	continue
    fi
    
    # e.g. line=group.phys-higgs.mc16_13TeV.410472.CAOD_HIGG5D2.e6348_e5984_s3126_r10201_r10210_p3639.32-07-3-2
    if [[ ${DEBUG} == "1" ]]; then
	echo "line=${line}"
    fi
    
    # split the line in two, the stem and vtag
    # e.g. stem=group.phys-higgs.mc16_13TeV.410472.CAOD_HIGG5D2.e6348_e5984_s3126_r10201_r10210_p3639
    # e.g. vtag=32-07-3-2
    stem="${line%.*}"
    vtag="${line##*.}"
    if [[ ${DEBUG} == "1" ]]; then
	echo "stem=${stem}"
	echo "vtag=${vtag}"
    fi

    # search for the stem in the ${INITIAL}
    line_initial=$(grep ${stem} ${INITIAL})
    if [[ ${line_initial} != "" ]]; then
	# if found replace the line found (irrespective of its vtag_initial) to our new desired vtag
	# which means replace line_initial with line
	if [[ ${DEBUG} == "1" ]]; then
	    echo "stem=${stem} found, and line_intial=${line_initial}"
	fi
	# replace line_initial with line
	COMMAND="sed -i -e 's|${line_initial}|${line}|g' ${INITIAL}"
	if [[ ${DEBUG} == "1" ]]; then
	    echo "COMMAND=${COMMAND}"
	fi
	eval ${COMMAND}
    else
	# if not found, append the new line, and at the end sort INITIAL
	if [[ ${DEBUG} == "1" ]]; then
	    echo "stem=${stem} not found"
	fi
	echo "${line}" >> ${INITIAL}
    fi
    # sort the output to have the DSID in order, to be easier for diff to spot the changes
    sort -o ${INITIAL} ${INITIAL}

    # done for one line    
done < ${REPLACEMENT}
# done loop over all the lines in ${REPLACEMENT}

# #########################################################################################################
# Done all                                                                                          #######
# #########################################################################################################

echo "Done updateOutputSampleList.sh for INITIAL=${INITIAL} and REPLACEMENT=${REPLACEMENT}!"
exit
