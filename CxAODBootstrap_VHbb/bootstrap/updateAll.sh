#!/bin/bash
PKG=CxAODBootstrap_VHbb
bsdir=$PKG/bootstrap
gitpackages=$bsdir/packages_VHbb_git.txt  
release=$(cat $bsdir/release.txt)

if [ "$1" = "-h" ] || [ "$1" = "--help" ] ; then
    echo 
    echo "$PKG update script. Prerequisites: ATLAS software setup e.g. setupATLAS "
    echo 
    echo "Usage: $0 [option...]" 
    echo
    echo "   no option (default)        install release/packages, copy files from afs and build everything from scratch (also for release change)"
    echo "    1                         update packages and afs files only and build (assume a release is already setup using asetup)"
    echo "                              Note: in case of release change it is recommended to start in a new shell, remove build direcory and call"
    echo "                              this script without an argument"           
    echo "   -h, --help                 this help " 
    echo
    kill -INT $$
fi
##############################################################
#
# Follow README instructions
#
#
# input parameters 
# none    setup from scratch
# 1       update packages
# 2       update packages and release 

input=$1

if [ "$input" = "" ] ; then
    echo "$PKG setup: Installing release/packages from scratch"
elif [ "$input" = "1" ] ; then
    echo "$PKG setup: Updating packages only"
elif [ "$input" = "2" ] ; then
    echo "$PKG setup: Updating packages and release"
else
    echo "input error"
fi 

echo "Using release" $release
echo "Using packages file" $gitpackages


# Setup athena for installing from scratch or requested release update
if [ "$input" = "" ] || [ "$input" = "2" ] ; then
    echo "command for unsetting previous release?"
    if [ ! -d ../build ] ; then 
	mkdir ../build
    fi
    cd ../build
    asetup AnalysisBase,$release,here
    cp CMakeLists.txt ../source    
fi
cd $TestArea/../source

# git checkout 
lsetup git
# http or ssh
#gitprefix=https://:@gitlab.cern.ch:8443
gitprefix=ssh://git@gitlab.cern.ch:7999
# loop over gitpackages file
while read line; do 
    # echo $line
    # ignore lines containing #
    if [ -z "$line" ]; then 
	continue 
    fi
    [[ "$line" =~ ^#.*$ ]] && continue

    pkg=$(echo ${line} | awk -F ' ' '{print $1}')
    repo=$(echo ${line} | awk -F ' ' '{print $2}')
    rev=$(echo ${line} | awk -F ' ' '{print $3}')
    echo 'Package repo revision' $pkg $repo $rev

    # try to clone packages unless the directory of the same name already exists
    # echo $pkg
    if [ ! -d $pkg ] ; then 
	echo "executing: git clone $gitprefix/$repo/$pkg.git"  
	git clone $gitprefix/$repo/$pkg.git
    fi
# update git packages to version in packages file 	
    if [ -d $pkg ] ; then 
	cd $pkg
	echo 'Directory is' `pwd`
	echo "executing: git checkout $rev -b ${rev}-${USER}"
	# we checkout the branch and put the username in the copied branch (similar to CxAOD)
	git checkout $rev -b ${rev}-${USER}	
	git fetch
	git merge
	cd ../
    fi
#    echo `pwd`
done < $gitpackages

# Now the data files are stored on afs use copy script
# Only copies files for packages you have checked out 
source $bsdir/copydatafromafs.sh ""  # default VHbb packages 

# Build 
echo "About to build"
cd $TestArea
# always cmake - in case new packages were added
cmake ../source
# Always build
cmake --build $TestArea 
# and set up the environment parameters
source */setup.sh

# and go back to source directory
echo "Build done, back to source directory, working directory is:"
cd ../source
pwd
