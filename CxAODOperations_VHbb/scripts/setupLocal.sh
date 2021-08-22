#!/usr/bin/env bash
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 == "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script ($0) forcing you to source. Don't execute me!" >&2
    exit 1
fi

# check if we have set up the CxAOD
if [ -z "$ISCXAODSETUP" ]; then
    echo "Variable ISCXAODSETUP is not set up, so we set up the CxAOD automatically for you"
    cd ../build
    pwd
    asetup --restore
    source x86_64-*-gcc*-opt/setup.sh
    cd ../run
else
   echo "Variable ISCXAODSETUP is set up, so we not set it up again"
fi

echo "Start to double check that everything is set up as needed"

# check if the CxAODFramework has been set up
if [[ ${ALRB_availableTools} == "" ]]; then
    echo "Cannot find asetup, as ALRB_availableTools is empty. Did you do setupATLAS?"
    return 1
fi
#
if [[ ${ALRB_availableTools} != *"asetup"* ]]; then
    echo "Cannot find asetup, as ALRB_availableTools is not empty, but does not contain asetup. Did you do setupATLAS?"
    return 1
fi
echo "ALRB_availableTools=${ALRB_availableTools}"
#
if [[ ${AtlasVersion} == "" ]]; then
    echo "AtlasVersion is not set. Did you do asetup?"
    return 1
fi
echo "AtlasVersion=${AtlasVersion}"
#
if [[ ${WorkDir_DIR} == "" ]]; then
    echo "WorkDir_DIR is not set. Did you do: cd build && source x86_64-*-gcc*-opt/setup.sh ?"
    return 1
fi

echo "Since we are here, CxAOD is well set up and WorkDir_DIR is defined as:"
echo "WorkDir_DIR=${WorkDir_DIR}"

# set the result that CxAOD is set up
ISCXAODSETUP="1"
echo "Finished setupLocal.sh"