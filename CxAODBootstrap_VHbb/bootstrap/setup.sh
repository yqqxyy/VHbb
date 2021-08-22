#!/usr/bin/env bash 
#if there is no parameter, it stops and it gives the instructions

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 == "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script forcing you to source. Don't execute me!" >&2
    exit 1
fi

if [[ $# -ne 2 ]]; then
cat <<EOF
Usage: #0 ${BRANCH_OR_TAG_NAME} ${BRANCH_OR_TAG_TYPE}
Usage: #0 origin/master         branch
Usage: #0 r30-05                tag
EOF
return
fi

BRANCH_OR_TAG_NAME=$1
BRANCH_OR_TAG_TYPE=$2
echo "BRANCH_OR_TAG_NAME=${BRANCH_OR_TAG_NAME}"
echo "BRANCH_OR_TAG_TYPE=${BRANCH_OR_TAG_TYPE}"

DATA_PREFIX="/afs/cern.ch/work/v/vhbbframework/public/data/dataForCxAODFramework_190601"
package="CxAODBootstrap_VHbb"
BOOTSTRAPDIR="${package}/bootstrap"

echo "DATA_PREFIX=${DATA_PREFIX}"
echo "package=${package}"
echo "BOOTSTRAPDIR=${BOOTSTRAPDIR}"

# 1. setup ATLAS and load a newer version of git
echo "STEP 1: setup ATLAS and load a newer version of git"
setupATLAS
lsetup git

# 2. GitLab rules do not allow to store binaries
# so *.root had been removed from data folder
# and are copied now from /afs to data folder
# ensure that MY_USER is set correctly 
echo "STEP 2: Set MY_USER to be able to copy .root from /afs"
#
if [[ -n "${CERN_USER}" ]]
then
    MY_USER=${CERN_USER}
    echo "You have defined CERN_USER, so using that for the scp."
else
    MY_USER=${USER}
    echo "You have not defined CERN_USER, so using USER. Define CERN_USER if your lxplus username is different than that of your local machine (for example at your home institute)"
fi
echo "MY_USER=${MY_USER}"
#
pwd

# 3. check out git packages and their .root files
echo "STEP 3: checkout git packages"
COMMAND="${BOOTSTRAPDIR}/checkout_packages_git.sh ${MY_USER} ${BOOTSTRAPDIR}/packages_VHbb_git.txt ${BRANCH_OR_TAG_NAME} ${BRANCH_OR_TAG_TYPE} 0 ${DATA_PREFIX}"
echo "COMMAND=${COMMAND}"
if ! eval ${COMMAND} ; then
    echo "problem checking out git packages" >&2
    return
fi
