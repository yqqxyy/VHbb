#!/usr/bin/env bash 
# created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the VH(bb) analysis group and CxAOD framework.
# you want to update the sample lists, so you will run the following steps one after the other
# so let's put them in one script, so that you can let it run overnight or when you do other work

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 != "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script forcing you to execute. Don't source me!" >&2
    return 1
else
    # if I am OK to execute, force that the script stops if variables are not defined
    # this catches bugs in the code when you think a variable has value, but it is empy
    set -eu
fi

# if there is no parameter, it stops and it gives the instructions
if [ $# -ne 0 ]; then
cat <<EOF
Usage: $0 
EOF
exit 1
fi

echo ""
echo "Start updateSampleInfo.sh"

# assumes you run from CxAODOperations_VHbb/data/CxAOD/CxAOD_31-15

# update the initial sample list
# e.g. list_sample_grid.mc16a_13TeV_25ns_signal_nominal.HIGG2D4.txt
# e.g. list_sample_grid.mc16a_13TeV_25ns_sigDBL_nominal.HIGG2D4.txt
# e.g. list_sample_grid.data15_13TeV_25ns.HIGG2D4.txt 
# e.g. list_sample_grid.data16_13TeV_25ns.HIGG2D4.txt 
# e.g. list_sample_grid.data17_13TeV_25ns.HIGG2D4.txt 
../../../scripts/createSampleList.py

# check if the new added samples have defined eos folder and cross section
../../../scripts/checkSubmissionWorkspace.py

# split in nominal and alternative, only after confirmed each MC DSID is in sample_info.txt 
../../../scripts/splitSampleList.py mcdata a,c,d HIGG5D1,HIGG5D2,HIGG2D4

# steps below are only done for MC

# for a given derivation, study the number of events in mc16a, mc16c, and mc16d
# to allow a comparison afterwards
# this step is slow as it retrieves info from ami
# e. g. summary_mc16_HIGG5D1.txt
../../../scripts/studySampleList.py 

# compare for a given derivation mc16a, mc16c, mc16d
# e.g summary_mc16_sum_HIGG5D1.txt
# and create the list of MC samples to use
# if a, use a; if d use d; if not d, use c.
# e.g. list_sample_grid.mc16a_used.HIGG5D1.txt 
# e.g. list_sample_grid.mc16c_used.HIGG5D1.txt 
# e.g. list_sample_grid.mc16d_used.HIGG5D1.txt 
../../../scripts/compareSampleList.py

# create the used_VHbb lists of DAOD if we want to run only on nominal samples
# e.g. list_sample_grid.mc16a_used_VHbb.HIGG5D1.txt
# e.g. list_sample_grid.mc16a_used_VHbb.HIGG5D2.txt
# e.g. list_sample_grid.mc16a_used_VHbb.HIGG2D4.txt
../../../scripts/createSampleListAnalysis.py

# create sample list of unique all DAOD, AOD and PRW
# e.g. list_sample_grid.mc16a_all.HIGG5D1.txt
# e.g. list_sample_grid.mc16a_all.HIGG5D2.txt
# e.g. list_sample_grid.mc16a_all.HIGG2D4.txt
# e.g. list_sample_grid.AOD.mc16a.txt
# e.g. list_sample_grid.PRW.mc16a.txt
../../../scripts/createSampleListPRW.py mc16a,mc16c,mc16d

# download the PRW files 
../../../scripts/downloadPRW.py /data06/abuzatu/data/PRW/180316 mc16a,mc16c,mc16d

echo ""
echo "Finished well updateSampleInfo.sh"
