#!/usr/bin/env bash
# if there is no parameter, it stops and it gives the instructions
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group 

[[ $- == *i* ]] && echo "Shell Interactive" || echo "Shell Not interactive"
if [[ $0 == "$BASH_SOURCE" ]]; then
    echo "ERROR: I'm a script ($0) forcing you to source. Don't execute me!" >&2
    exit 1
fi

echo "************************************************************************"
echo "**************** setupATLAS ********************************************"
echo "************************************************************************"
setupATLAS
echo "************************************************************************"
echo "**************** setup ROOT ********************************************"
echo "************************************************************************"
lsetup root
echo "************************************************************************"
echo "**************** setup Rucio *******************************************"
echo "************************************************************************"
lsetup rucio
#echo "************************************************************************"
#echo "**************** get voms proxy ****************************************"
#echo "************************************************************************"
voms-proxy-init -voms atlas -valid 96:0
echo "************************************************************************"
echo "**************** setup PyAmy *******************************************"
echo "************************************************************************"
lsetup pyami
echo "************************************************************************"
echo "**************** setting up ISDOWNLOADSETUP=1 **************************"
echo "************************************************************************"
export ISDOWNLOADSETUP=1

# to read the dataset from a cross section, along with other info
# ami show dataset info mc15_13TeV.361596.PowhegHerwigppEG_CT10nloME_AZNLOCTEQ6L1_ZZqqll_mqq20mll20.merge.DAOD_HIGG5D2.e4769_s2726_r7772_r7676_p2949
# examples at https://ami.in2p3.fr/pyAMI/command_line.html
# other
# to make eos reading in Reader available at Glasgow
# source /afs/cern.ch/project/eos/installation/atlas/etc/setup.sh
# export EOS_MGM_URL=${EOS_MGM_URL-"root://eosatlas.cern.ch"}
