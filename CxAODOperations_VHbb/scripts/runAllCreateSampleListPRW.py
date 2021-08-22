#!/usr/bin/env python
# Created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis
# to run in parallel createSampleListPRW.py for each option, channel, mcType

# read command line arguments
import sys
import os
# run and process output of bash commands
import subprocess


total = len(sys.argv)
# number of arguments plus 1
if total!=4:
    print "You need some arguments, will ABORT!"
    print "Ex: ",sys.argv[0]," option       channel  mcType"
    print "Ex: ",sys.argv[0]," mcdata       0L,1L,2L a,d"
    print "Ex: ",sys.argv[0]," mcdataVHbb   0L,1L,2L a,d"
    assert(False)
# done if

#################################################################
################### Configurations ##############################
#################################################################

debug=False
list_option=sys.argv[1].split(",")
list_channel=sys.argv[2].split(",")
list_mcType=sys.argv[3].split(",")
if debug:
    print "list_option",list_option
    print "list_channel",list_channel
    print "list_mcType",list_mcType

dict_channel_derivation={
    "0L":"HIGG5D1",
    "1L":"HIGG5D2",
    "2L":"HIGG2D4",
    }

#################################################################
################### Functions ###################################
#################################################################

def doIt():
    for option in list_option:
        for channel in list_channel:
            derivation=dict_channel_derivation[channel]
            for mcType in list_mcType:
                command="nohup ../../../CxAODOperations_VHbb/scripts/createSampleListPRW.py "+option+" "+derivation+" "+mcType+" >& run_createSampleListPRW_"+option+"_"+channel+"_"+mcType+".log &"
                print "command="+command
                os.system(command)
            # done loop over mcType
        # done loop over channel
    # done for loop over option
# done if

#################################################################
################### Run #########################################
#################################################################

doIt()

#################################################################
################### Finished ####################################
#################################################################
print ""
print ""
print "Finished all in runAllCreateSampleListPRW.py."
