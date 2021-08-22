#!/usr/bin/env bash
# git clone -b FrameworkSub ssh://git@gitlab.cern.ch:7999/CxAODFramework/FrameworkSub.git

# forbids an interactive shell from running this executable, in other words, do not source
if [[ $- == *i* ]] ; then
    echo "ERROR: I'm a script forcing you to execute. Don't source me!" >&2
    return 1
else
    # if I am OK to execute, force that the script stops if variables are not defined
    # this catches bugs in the code when you think a variable has value, but it is empy
    set -eu
fi

# check the number of parameters, if not stop
if [[ $# -ne 6 ]]; then
cat <<EOF
Usage: $0 MY_USER PACKAGE_LIST          BRANCH_OR_TAG_NAME BRANCH_OR_TAG_TYPE FORCE_CHECKOUT DATA_PREFIX
Usage: $0 $USER   packages_VHbb_git.txt origin/master      branch             0              /afs/cern.ch/work/v/vhbbframework/public/data/dataForCxAODFramework
Usage: $0 $USER   packages_VHbb_git.txt r31-01             tag                0              /afs/cern.ch/work/v/vhbbframework/public/data/dataForCxAODFramework
Usage: WARNING:   Use CERN_USER if your local machine has a different username USER than the one on lxplus (to scp correctly the .root files, not in GitLab any more!)
EOF
exit 1
fi

MY_USER=$1
PACKAGE_LIST=$2
BRANCH_OR_TAG_NAME=$3
BRANCH_OR_TAG_TYPE=$4
FORCE_CHECKOUT=$5
DATA_PREFIX=$6
PATH_PREFIX="ssh://git@gitlab.cern.ch:7999"

echo "MY_USER=${MY_USER}"
echo "PACKAGE_LIST=${PACKAGE_LIST}"
echo "BRANCH_OR_TAG_NAME=${BRANCH_OR_TAG_NAME}"
echo "BRANCH_OR_TAG_TYPE=${BRANCH_OR_TAG_TYPE}"
echo "FORCE_CHECKOUT=${FORCE_CHECKOUT}"
echo "DATA_PREFIX=${DATA_PREFIX}"
echo "PATH_PREFIX=${PATH_PREFIX}"

# start loop over CxAODFramework packages
while read line
do

    # skip coments and blank lines
    QUOTE_RE="^#"
    EMPTY_RE="^$"
    if [[ $line =~ $QUOTE_RE || $line =~ $EMPTY_RE ]] ; then
        continue
    fi

    #
    package=$(echo "${line}" | awk '{print $1}')        # package name
    repo=$(echo "${line}" | awk '{print $2}')           # repo name
    branch_or_tag=$(echo "${line}" | awk '{print $3}')  # note the default is for branches
    copy_data_root=$(echo "${line}" | awk '{print $4}') # if we copy from /afs the data/*.root for this package 
    path=${PATH_PREFIX}/${repo}/${package}.git
    echo "repository=${repo} package=${package} branch_or_tag=${branch_or_tag} path=${path}"
    echo "copy_data_root=${copy_data_root}"
    #
    if [[  -d ${package} ]] ; then
        if [[ ${FORCE_CHECKOUT} == "1" ]] ; then
            rm -rf ${package}
        else
            echo "$path already checked out"
            continue
        fi
    fi
    
    # clone the entire package
    COMMAND="git clone ${path}"
    echo "COMAND=${COMMAND}"
    eval ${COMMAND}
    
    # if folder does not exist, continue
    if [[ ! -d "${package}" ]]; then
	echo "WARNING!!! package ${package} does not have a folder! We'll skip the checkout of a branch_or_tag for this package!"
	continue
    fi

    # cd to that package
    COMMAND="cd ${package}"
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}

    # git checkout depending if it is a branch or a tag
    # if the user asked for FrameworkSub to be a branch, check them out exactly as they are
    # meaning the branches given for the CxAOD packages 
    # and the tag given for those that have a tag, like the  NNLOReweighter
    # if the user asked for FrameworkSub to be a given tag,
    # check the same tag for the CxAOD packages
    # but keep the same tag for those that already had a tag, like the NNLOReweighter
    echo "Doing git checkout with -b, so that you have a branch to develop on directly."
    echo "So you can commit and merge directly. This for both branch and tag."
    branch_specific="origin/"
    if [[ "${branch_or_tag}" == "${branch_specific}"* ]]; then
	echo "You asked for a branch, as it starts with ${branch_specific}."
	branch_name=${branch_or_tag#${branch_specific}}
	echo "branch_or_tag=${branch_or_tag} branch_name=${branch_name}"
	if [[ "${BRANCH_OR_TAG_TYPE}" == "branch" ]] ; then
	    COMMAND="git checkout ${branch_or_tag} -b ${branch_name}-${MY_USER}"
	elif [[ "${BRANCH_OR_TAG_TYPE}" == "tag" ]]; then
	    COMMAND="git checkout ${BRANCH_OR_TAG_NAME} -b master-${BRANCH_OR_TAG_TYPE}-${MY_USER}"
	else
	    echo "BRANCH_OR_TAG_TYPE=${BRANCH_OR_TAG_TYPE} is neither branch or tag. Will ABORT!!!"
	    exit -1
	fi
    else
	echo "You asked for a tag, as is it does not start with ${branch_specific}"
	tag_name=${branch_or_tag}
	echo "branch_or_tag=${branch_or_tag} tag_name=${tag_name}"
	COMMAND="git checkout ${branch_or_tag} -b master-${tag_name}-${MY_USER}"
    fi
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}

    # GitLab rules do not allow to store binaries
    # so *.root had been removed from data folder
    # and are copied now from /afs to data folder
    COMMAND="pwd ; ls -lha ; du -sh *"
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}
    if [[ "${copy_data_root}" == "1" ]]; then
	if [[ ! -d ${DATA_PREFIX} ]] ; then
	    echo "You can not read directly the /afs folders, so we need with scp."
	    COMMAND="scp -r ${MY_USER}@lxplus.cern.ch:"
	else
	    echo "You have access to /afs, so copy simply from there cp."
	    COMMAND="cp -r "
	fi
	COMMAND+="${DATA_PREFIX}/${package}/* data/."
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi
    COMMAND="ls -lha data"
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}

    # return to previous folder
    COMMAND="cd .."
    echo "COMMAND=${COMMAND}"
    eval ${COMMAND}

    # done all for current package
done < ${PACKAGE_LIST}
# done loop over all the packages

echo "Done the checkout of git packages!"
