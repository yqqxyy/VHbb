#!/bin/bash
#if there is no parameter, it stops and it gives the instructions
if [ $# -ne 4 ]; then
cat <<EOF
Usage: $0 BRANCH_OR_TAG_NAME     FOLDER                                    DO_PACKAGES DO_BUILD
Usage: $0 origin/master          CxAODFramework_branch_master_21.2.89_1    1           1
Usage: $0 r32-23                 CxAODFramework_tag_r32-23                 1           1
BRANCH_OR_TAG_NAME: You can decide which branch name or which tag name to check out. 
r30-10 is the first tag of the master after dev-R21 -> master.
Some user can create one's own BOOTSTRAP_REPOSITORY branch that checks out particular branch values for each of the other packages.
Or we can check out one tag, or another tag.
The branch has to start with "origin/".
The tag has to start with "r".
FOLDER: choose the folder that will contain all the CxAODFramework packages.
DO_PACKAGES=0 to only check out BOOTSTRAP_REPOSITORY and stop.
DO_PACKAGES=1 and DO_BUILD=0 to check out all the packages, then stop
DO_PACKAGES=1 and DO_BUILD=1 to check out all packages, setup AB and compile.
EOF
return
fi

BRANCH_OR_TAG_NAME=$1
FOLDER=$2
DO_PACKAGES=$3
DO_BUILD=$4

echo "BRANCH_OR_TAG_NAME=${BRANCH_OR_TAG_NAME}"
echo "FOLDER=${FOLDER}"
echo "DO_PACKAGES=${DO_PACKAGES}"
echo "DO_BUILD=${DO_BUILD}"

############################################################################
### check if BRANCH_OR_TAG_TYPE is not of type branch or tag, then abort ###
############################################################################

BRANCH_PREFIX="origin/"
BOOTSTRAP_REPOSITORY_VERSION="0"

if [[ ${BRANCH_OR_TAG_NAME} == ${BRANCH_PREFIX}* ]]; then
    echo "You asked to checkout a branch, as it starts with \"origin/\". We continue correctly."
    BRANCH_OR_TAG_TYPE="branch"
    PREFIX=""
    STEM=${BRANCH_OR_TAG_NAME#${BRANCH_PREFIX}}
    BOOTSTRAP_REPOSITORY_VERSION="2"
elif [[ ${BRANCH_OR_TAG_NAME} == r* ]]; then
    echo "You asked to checkout a tag, as it starts with \"r\". We continue correctly."
    BRANCH_OR_TAG_TYPE="tag"
    PREFIX="master-"
    STEM="${BRANCH_OR_TAG_NAME}"
    # STEM is of naming convention r31-15, so we can find the value of TAG and VERSION
    TAG=${STEM:1:2} # e.g. 31
    VERSION=${STEM:4:6} # e.g. 15
    echo "TAG=${TAG} VERSION=${VERSION}"
    if [[ ${TAG} < "30" ]]; then 
	echo "Tag earlier than 30 is not supported by this bootstrap script, as it was before R21"
	return -1
    elif [[ ${TAG} == "30" ]]; then
	if [[ ${VERSION} < "10" ]]; then
	    echo "Tag 30 with version earlier than 10 is before we moved from dev-R21 to master, so not supported by this boostrap script"
	    return -1
	else
	    BOOTSTRAP_REPOSITORY_VERSION="1"
	fi
    elif [[ ${TAG} == "31" ]]; then
	if [[ ${VERSION} < "18" ]]; then
	    BOOTSTRAP_REPOSITORY_VERSION="1"
	else
	    BOOTSTRAP_REPOSITORY_VERSION="2"
	fi
    else
	BOOTSTRAP_REPOSITORY_VERSION="2"
    fi
else
    echo "You asked for BRANCH_OR_TAG_NAME=${BRANCH_OR_TAG_NAME}. But it must be of type branch (starting with origin/) or tag (to start with r). Will ABORT!!!"
    return
fi

echo "BRANCH_OR_TAG_TYPE=${BRANCH_OR_TAG_TYPE}"
echo "PREFIX=${PREFIX}"
echo "STEM=${STEM} TAG=${TAG} VERSION=${VERSION}"
if [[ ${BOOTSTRAP_REPOSITORY_VERSION} == "1" ]]; then
    BOOTSTRAP_REPOSITORY="FrameworkSub"
    DATA_PREFIX="/afs/cern.ch/work/v/vhbbframework/public/data/dataForCxAODFramework"
elif [[ ${BOOTSTRAP_REPOSITORY_VERSION} == "2" ]]; then
    BOOTSTRAP_REPOSITORY="CxAODBootstrap_VHbb"
    DATA_PREFIX="/afs/cern.ch/work/v/vhbbframework/public/data/dataForCxAODFramework_181026"
else
    echo "BOOTSTRAP_REPOSITORY_VERSION=${BOOTSTRAP_REPOSITORY_VERSION} not known. We need 1 or 2. Will ABORT!!!"
    return -1
fi
echo "BOOTSTRAP_REPOSITORY=${BOOTSTRAP_REPOSITORY}"
echo "DATA_PREFIX=${DATA_PREFIX}"

####################################################################################################
### checkout out the CxAOD framework, first the bootstrap package, and from there the rest #########
####################################################################################################

# check out BOOTSTRAP_REPOSITORY
mkdir ${FOLDER}
cd ${FOLDER}
mkdir source build run
# in the run folder, create the folders configs, logs and the file clean.sh
cd run
mkdir configs
mkdir logs
touch clean.sh
echo "rm -rf Maker_*" >> clean.sh
echo "rm -f logs/*.log" >> clean.sh
echo "rm -f *.log" >> clean.sh
echo "rm -f *.txt" >> clean.sh
echo "rm -f configs/*.cfg" >> clean.sh
echo "rm -f *.root" >> clean.sh
chmod +x clean.sh
cd ..
# check out the other packages in source
cd source
COMMAND="pwd; git clone ssh://git@gitlab.cern.ch:7999/CxAODFramework/${BOOTSTRAP_REPOSITORY}.git"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}
cd ${BOOTSTRAP_REPOSITORY}
COMMAND="pwd; git checkout ${BRANCH_OR_TAG_NAME} -b ${PREFIX}${STEM}-${USER}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}
cd ..
# decide if to stop or go on to the rest of the packages
if [[ "${DO_PACKAGES}" == "0" ]] ; then
    echo "We stop after having checked out ${BOOTSTRAP_REPOSITORY} with BRANCH_OR_TAG_NAME=${BRANCH_OR_TAG_NAME} and BRANCH_OR_TAG_TYPE=${BRANCH_OR_TAG_TYPE},"
    echo "and before starting to check out the rest of the packages"
    return
fi
# print the list of packages
COMMAND="cat ./${BOOTSTRAP_REPOSITORY}/bootstrap/packages_VHbb_git.txt"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}
# check out code and .root for the rest of the packages
COMMAND="source ./${BOOTSTRAP_REPOSITORY}/bootstrap/setup.sh ${BRANCH_OR_TAG_NAME} ${BRANCH_OR_TAG_TYPE}"
echo "COMMAND=${COMMAND}"
eval ${COMMAND}
# asetup and build
if [[ "${DO_BUILD}" == "0" ]] ; then
    echo "We stop after having checked out all the packages with BRANCH_OR_TAG_NAME=${BRANCH_OR_TAG_NAME} and BRANCH_OR_TAG_TYPE=${BRANCH_OR_TAG_TYPE},"
    echo "and before starting to asetup and build the packages"
    return
fi
echo "Going to build folder"
cd ../build
echo "Setting up ATLAS"
setupATLAS
echo "Setting up the AnalysisBase release"
lsetup asetup
release=`cat ../source/${BOOTSTRAP_REPOSITORY}/bootstrap/release.txt`
echo "release=$release"
asetup AnalysisBase,$release,here
echo "Starting to build/compile"
cp CMakeLists.txt ../source
cmake ../source
cmake --build .
echo "Finished compilation, so doing source x86_64-slc6-gcc62-opt/setup.sh"
source x86_64-*-gcc*-opt/setup.sh
# echo "Setting up a version of PyRoot that contains numpy, allowing us to read CxAOD files in PyRoot"
# lsetup 'lcgenv -p LCG_91 x86_64-slc6-gcc62-opt numpy' # must be explicitely said, wihout * as in th source above

####################################################################################################
### Run the test locally for many samples, same as in Continuous Integration pipeline in GitLab  ###
####################################################################################################

echo "Going to the run folder"
cd ../run
pwd
echo "You can now test the Maker or Reader"
if [[ ${BOOTSTRAP_REPOSITORY_VERSION} == "2" ]]; then
    echo "For Maker, you can test in one go for all possible cases with"
    echo "source ../source/CxAODOperations_VHbb/scripts/testLocallyAsInCIPipeline.sh"
    echo "To make this easier for you to run many times as you develop, we create an alias t to this command"
    COMMAND="alias t=\"source ../source/CxAODOperations_VHbb/scripts/testLocallyAsInCIPipeline.sh testLocallyAsInCIPipelineTasks.txt\""
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}
    echo ""
    echo "Careful as this will launch many jobs in parallel, and in lxplus even one job is slow, all jobs is hopeless!"
    echo "To see which jobs will be run, open this file"
    echo "emacs -nw ../source/CxAODOperations_VHbb/data/DxAOD/info/testLocallyAsInCIPipeline.txt"
    echo "Comment out those that you do not want to be run, so that you can test only the one you want"
    echo "source ../source/CxAODOperations_VHbb/scripts/testLocallyAsInCIPipeline.sh testLocallyAsInCIPipelineTasks.txt"
    echo "If you are on lxplus, there would be too many jobs to run in parallel, so let's run a reduced number"
    echo "source ../source/CxAODOperations_VHbb/scripts/testLocallyAsInCIPipeline.sh testLocallyAsInCIPipelineTasksReduced.txt"
    echo "and the new test samples with patch for MET Trigger skimming in 0L and 1L"
    echo "source ../source/CxAODOperations_VHbb/scripts/testLocallyAsInCIPipeline.sh testLocallyAsInCIPipelineTasks2.txt"
    echo ""
    echo "But you can also run on just one sample locally directly with submitMaker.sh"
    echo "Run without any arguments, and it will show you the arguments to run out of the box with copy/paste from the command line arguments shown"
    echo "source ../CxAODOperations_VHbb/scripts/submitMaker.sh"
    echo "To make this easier for you to run many times as you develop, we create an alias m to this command"
    COMMAND="alias m=\"source ../CxAODOperations_VHbb/scripts/submitMaker.sh\""
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}
    echo "After every test, if needed, you can/shoud run ./clean.sh to remove the files produced (Maker, .root, configs, logs)."
    echo "To submit to the grid, from the same place, with other examples from the submitMaker.sh"
    echo ""
    echo "You can also run the Reader from this folder, either locally or on a batch system"
    echo "source ../CxAODOperations_VHbb/scripts/submitReader.sh"
    echo "To make this easier for you to run many times as you develop, we create an alias r to this command"
    COMMAND="alias r=\"source ../CxAODOperations_VHbb/scripts/submitReader.sh\""
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}
    echo ""
    echo "CxAODFramework for VHbb (Maker+Reader) installed. Good luck exploring CxAOD!"
fi
