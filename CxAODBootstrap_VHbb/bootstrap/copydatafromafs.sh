#!/usr/bin/env bash 
if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
    echo 
    echo "Copy data binary files from CERN afs"
    echo "Usage: $0 [option...]" 
    echo
    echo "   no option (default)        Copy from main directory /afs/cern.ch/work/v/vhbbframework/public/data/dataForCxAODFramework"
    echo "   [directory]                Copy from user dirfined directory "  
    echo "   -h, --help                 this help "
    echo
    kill -INT $$
fi

# see if directory provided else take core packages as default
if [ "$1" != "" ]  ; then
    DATA_FILES=$1
else
    DATA_FILES="/afs/cern.ch/work/v/vhbbframework/public/data/dataForCxAODFramework"
fi
echo "Copying data files from" $DATA_FILES

# check if afs is visible - if not need to scp to lxplus using afs username
copycommand="cp -Lrp "
if [ ! -d $DATA_FILES ] ; then
    echo "Directory not visible - need kinit? Set afs username and use scp this time" 
    echo "Check if CERN afs username is set to CERN_USER"
    if [ -n "$CERN_USER" ]; then
	echo "CERN_USER is set to" $CERN_USER
    else
	read -p "CERN_USERname? " CERN_USER
	echo "CERN_USER is set to" $CERN_USER "use unset CERN_USER if entered incorrectly"
    fi
    copycommand="scp -p $CERN_USER@lxplus.cern.ch:"
fi

# 2. GitLab rules do not allow to store binaries
# so *.root had been removed from data folder
# and are copied now from /afs to data folder
packages=()
# put remote directories into an array
echo "Reading $DATA_FILES/"
for dir in $DATA_FILES/* ; do 
  if [[ -d "$dir" ]]; then
#      echo 'directory' $dir 
      packages+=($dir)
      bname=$(basename $dir)
      echo "Found remote package $bname"
  fi
done


echo "Copying directories for locally existing packages"
for i in "${packages[@]}"
do
#   echo "$i"
    bname=$(basename $i)
#   echo "$bname"
   # if the remote package exists locally copy the file
    if [[ -d "$bname" ]]; then
	COMMAND=$copycommand
	COMMAND+="${i}/* ${bname}/data/."
	echo "COMMAND=${COMMAND}"
	${COMMAND}
    fi
done

echo "Copying done"
echo "Note: To set the correct symlinks in ../build/x86_64-slc6-gcc62-opt/data/ you need the following build sequence:" 
echo 'cd $TestArea; cmake ../source; cmake --build .' 
