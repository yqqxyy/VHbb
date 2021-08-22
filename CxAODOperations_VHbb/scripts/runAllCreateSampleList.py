#!/usr/bin/env python
# Created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis
# to run in parallel createSampleList.py for each option, channel, mcType
# assumes to be run from the VHbb or for the DBL folder

# read command line arguments
import sys
import os
# run and process output of bash commands
import subprocess


total = len(sys.argv)
# number of arguments plus 1

if total!=5:
    print "You need some arguments, will ABORT!"
    print "Ex: ",sys.argv[0]," options              channels mcTypes CheckAMIAllEvents"
    print "Ex: ",sys.argv[0]," VHbb                 0L,1L,2L a,d,e   0"
    print "Ex: ",sys.argv[0]," VHbbWrongMETTrigger  0L,1L    e       0"
    print "Ex: ",sys.argv[0]," DBL                  0L,1L,2L a,d,e   0"
    print "Ex: ",sys.argv[0]," DBLPatch             0L,1L    e       0"
    assert(False)
# done if

#################################################################
################### Configurations ##############################
#################################################################

debug=False
list_option=sys.argv[1].split(",")
list_channel=sys.argv[2].split(",")
list_mcType=sys.argv[3].split(",")
checkInAMIAllEventsAreAvailableString=sys.argv[4]
if debug:
    print "list_option",list_option
    print "list_channel",list_channel
    print "list_mcType",list_mcType
    print "checkInAMIAllEventsAreAvailable",checkInAMIAllEventsAreAvailable

#################################################################
################### Functions ###################################
#################################################################

def doIt():
    for option in list_option:
        for channel in list_channel:
            for mcType in list_mcType:
                command="nohup ../../../scripts/createSampleList.py "+option+" "+channel+" "+mcType+" "+checkInAMIAllEventsAreAvailableString+" >& run_createSampleList_"+option+"_"+channel+"_"+mcType+".log &"
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
print "Finished all in runAllCreateSampleList.py"
