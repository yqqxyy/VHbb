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
if [ $# -ne 8 ]; then
cat <<EOF
Usage: $0 InputFolder                          OutputFolder                         Channel  MCType Model   Tag   VTags DoExecute
Usage: $0 /data06/abuzatu/data/Reader/180519_1 /data06/abuzatu/data/Reader/180519_3 0L,1L,2L a,d    MVA,CUT D,T,H 31-10 1
Usage: $0 /data06/abuzatu/data/Reader/180519_1 /data06/abuzatu/data/Reader/180519_3 0L       a,d    MVA,CUT D,T,H 31-10 1
Usage: $0 /data06/abuzatu/data/Reader/180519_2 /data06/abuzatu/data/Reader/180519_3 0L       a,d    MVA,CUT D,T,H 31-10 1 
Usage: most common user case is when you have submitted all samples, but one process crashed for some reason, or did not run at all (the _1).
Usage: you then resubmit those samples and then you have (_2)
Usage: you want to create _3 that has the inital from _1 and then _2. 
Usage: you can extend the usage to remove some samples, or rename, etc.
Usage: the important is that it gives you the loop over all the them.
EOF
exit 1
fi


FOLDER_INPUT=$1
FOLDER_OUTPUT=$2
CHANNELs=$3
MCTYPEs=$4
MODELTYPEs=$5
TAGGINGs=$6
VTAGs=$7
DO_EXECUTE=$8

echo ""
echo ""
echo ""
echo "Start submitReader.sh"
echo "FOLDER_INPUT=${FOLDER_INPUT}"
echo "FOLDER_OUTPUT=${FOLDER_OUTPUT}"
echo "CHANNELs=${CHANNELs}"
echo "MCTYPEs=${MCTYPEs}"
echo "MODELTYPEs=${MODELTYPEs}"
echo "TAGGINGs=${TAGGINGs}"
echo "VTAGs=${VTAGs}"
echo "VTAGs=${VTAGs}"
echo "DO_EXECUTE=${DO_EXECUTE}"

DEBUG="false"
for CHANNEL in `echo "${CHANNELs}" | awk -v RS=, '{print}'`
do
    for MCTYPE in `echo "${MCTYPEs}" | awk -v RS=, '{print}'`
    do
	for VTAG in `echo "${VTAGs}" | awk -v RS=, '{print}'`
	do
	    for TAGGING in `echo "${TAGGINGs}" | awk -v RS=, '{print}'`
	    do
		for MODELTYPE in `echo "${MODELTYPEs}" | awk -v RS=, '{print}'`
		do
		    FOLDER_STEM="Reader_${CHANNEL}_${MODELTYPE}_${TAGGING}_${VTAG}_${MCTYPE}"
		    echo "FOLDER_STEM=${FOLDER_STEM}"
		    FOLDER_INPUT_FETCH="${FOLDER_INPUT}/${FOLDER_STEM}/fetch"
		    FOLDER_OUTPUT_FETCH="${FOLDER_OUTPUT}/${FOLDER_STEM}/fetch"
		    echo "FOLDER_INPUT_FETCH=${FOLDER_INPUT_FETCH}"
		    echo "FOLDER_OUTPUT_FETCH=${FOLDER_OUTPUT_FETCH}"
		    mkdir -p ${FOLDER_OUTPUT_FETCH}	
		    COMMAND="cp -r ${FOLDER_INPUT_FETCH}/* ${FOLDER_OUTPUT_FETCH}/."
		    echo "COMMAND=${COMMAND}"
		    if [[ ${DO_EXECUTE} == "1" ]]; then
			eval ${COMMAND}
		    fi
		done
	    done
	done
    done
done
