#!/usr/bin/env bash
# if there is no parameter, it stops and it gives the instructions
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 != "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script ($0) forcing you to execute. Don't source me!" >&2
    return 1
else
    # if I am OK to execute, force that the script stops if variables are not defined
    # this catches bugs in the code when you think a variable has value, but it is empty
    set -eu
fi

# if there is no parameter, it stops and it gives the instructions
if [ $# -ne 0 ]; then
cat <<EOF
Usage: $0
EOF
exit 1
fi

# #########################################################################################################
# Configs                                                                                           #######
# #########################################################################################################

SUFFIX="_CxAOD.root"
OUT_SAMPLE_FILE="out_sample_list_sample_grid.13TeV_25ns.mcdata_a.HIGG5D1.txt"
echo "OUT_SAMPLE_FILE=${OUT_SAMPLE_FILE}"

# #########################################################################################################
# Run                                                                                               #######
# #########################################################################################################

echo ""
echo ""

if [ -f ${OUT_SAMPLE_FILE} ]; 
then
    TIME_STAMP="$(date +%Y%m%d_%H%M%S)"
    echo "TIME_STAMP=${TIME_STAMP}"
    OUT_SAMPLE_FILE_BACKUP="${OUT_SAMPLE_FILE}.${TIME_STAMP}"
    echo "OUT_SAMPLE_FILE_BACKUP=${OUT_SAMPLE_FILE_BACKUP}"
    mv ${OUT_SAMPLE_FILE} ${OUT_SAMPLE_FILE_BACKUP}
fi

for SAMPLE_FOLDER in $(find . -name "*_CxAOD.root") ; 
do 
    echo "SAMPLE_FOLDER=${SAMPLE_FOLDER}"
    SAMPLE_FOLDER=$(basename ${SAMPLE_FOLDER})
    SAMPLE_FOLDER_STEM=${SAMPLE_FOLDER%$SUFFIX}
    # echo "SAMPLE_FOLDER_STEM=${SAMPLE_FOLDER_STEM}"
    echo "${SAMPLE_FOLDER_STEM}" >> ${OUT_SAMPLE_FILE}
done

# now need to sort alphabetically
sort ${OUT_SAMPLE_FILE} > ${OUT_SAMPLE_FILE}_sorted
mv ${OUT_SAMPLE_FILE}_sorted ${OUT_SAMPLE_FILE}

# #########################################################################################################
# Done all                                                                                          #######
# #########################################################################################################

echo ""
echo ""
echo "All finished well."
exit
