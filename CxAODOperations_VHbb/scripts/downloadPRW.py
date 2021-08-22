#!/usr/bin/env python   
# Created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis
# to download the PRW file (NTUP_PILEUP) from the PRW sample list we created

# read command line arguments
import os,sys
# run and process output of bash commands
import subprocess

total = len(sys.argv)
# number of arguments plus 1
if total!=3:
    print "You need some arguments, will ABORT!"
    print "Ex: ",sys.argv[0]," outputFolder                     list_mcType"
    print "Ex: ",sys.argv[0]," /data06/abuzatu/data/PRW/180313  mc16a,mc16c,mc16d"
    assert(False)
# done if

outputFolder=sys.argv[1]
list_mcType=sys.argv[2].split(",") # mc16a,mc16c,mc16d

#################################################################
################### Configurations ##############################
#################################################################

debug=False
verbose=True
doTest=False

#################################################################
################### Functions ###################################
#################################################################

def download(mcType):
    folderName=outputFolder+"/"+mcType
    os.system("mkdir -p "+folderName)
    inputFileName="list_sample_grid.PRW."+mcType+".txt"
    with open(inputFileName,'r') as inputFile:
        for line in inputFile:
            dataset=line.rstrip()
            if debug:
                print "dataset",dataset
            command="pushd "+folderName+"&& echo in folder: && pwd && rucio download "+dataset+" && popd"
            if debug:
                print "command",command
            os.system(command)
        # done loop over lines
    # close file
# done function

def doIt():
    for mcType in list_mcType:
        download(mcType)
# done function

#################################################################
################### Run #########################################
#################################################################

doIt()

#################################################################
################### Finished ####################################
#################################################################

print ""
print ""
print "Finished all fine."
