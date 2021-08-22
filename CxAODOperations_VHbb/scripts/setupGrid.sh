#!/usr/bin/env bash 
# if there is no parameter, it stops and it gives the instructions
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 == "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script ($0) forcing you to source. Don't execute me!" >&2
    exit 1
fi

# if there is no parameter, it stops and it gives the instructions
if [ $# -ne 2 ]; then
cat <<EOF
Usage: source $0 GridPrivilege DoExecute
Usage: source $0 Higgs         1
Usage: source $0 Exotics       1
Usage: source $0 Hdbs          1
Usage: source $0 user          1
EOF
return
fi

GRID_PRIVILEGE=$1
DO_EXECUTE=$2

# e.g. voms-proxy-init -voms atlas:/atlas/phys-higgs/Role=production -valid 96:0
# e.g. voms-proxy-init -voms atlas: -valid 96:0
# e.g. voms-proxy-init -voms atlas

echo "setupATLAS"
setupATLAS

NR_HOURS="6"
COMMAND="lsetup rucio && voms-proxy-info -exists -valid ${NR_HOURS}:00"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}
if [ $? -ne 0 ]; then
    echo "Proxy is valid for less than ${NR_HOURS} hours out of the original 96 hours, so we reset voms"
    DO_INIT_VOMS="1"
else
    DO_INIT_VOMS="0"
fi
echo "DO_INIT_VOMS=${DO_INIT_VOMS}"

COMMAND="voms-proxy-init -voms atlas"
if [[ ${GRID_PRIVILEGE} == "Higgs" ]]; then
    COMMAND+=":/atlas/phys-higgs/Role=production"
elif [[ ${GRID_PRIVILEGE} == "Exotics" ]]; then
    COMMAND+=":/atlas/phys-exotics/Role=production"
elif [[ ${GRID_PRIVILEGE} == "Hdbs" ]]; then
    COMMAND+=":/atlas/phys-hdbs/Role=production"
elif [[ ${GRID_PRIVILEGE} == "user" ]]; then
    COMMAND+=""
else
    echo "GRID_PRIVILEGE=${GRID_PRIVILEGE} is not known. Choose Higgs, Exotics, Hdbs or user. Will ABORT!!!"
    return
fi
COMMAND+=" -valid 96:0" # to be valid 4 days instead of 12 hours
echo "COMMAND=${COMMAND}"
if [[ ${DO_EXECUTE} != "1" ]]; then
    return
fi

echo "setup panda, rucio, pyami"
lsetup panda
lsetup rucio
lsetup pyami

if [[ ${DO_INIT_VOMS} == "1" ]]; then
    echo "Set up voms as a function of your grid privileges (Higgs, or Exotics, or Hdbs or (regular) user"
    eval ${COMMAND}
fi

# set the result that the grid is set up
ISGRIDSETUP="1"
echo "Finished setupGrid.sh"
