#!/usr/bin/env python
#
# Run getAMIInfo.py in parallel over list of samples of the form:
#
# [stemlist]_[periodMC]_[derivation]
#
# where stemlist='list_sample_grid.13TeV_25ns.mcdata' by default,
# periodMC=a,d,e 
# and derivation=HIGG5D1,HIGGD2,HIGG2D4  
# The derivations are selected using the channel steering=0L,1L,2L
#
# Warning: the script assumes that you are running in the VHbb or the DBL folder e.g.
# CxAODOperations_VHbb/data/DxAOD/VHbb
#
import sys
import os
import subprocess
import argparse

if __name__ == '__main__':

    parser = argparse.ArgumentParser()     
    parser.add_argument('-s','--stemlist', help='stem of the samples list', default='list_sample_grid.13TeV_25ns.mcdata')     
    parser.add_argument('-c','--channels', help='channels e.g. 0L,1L,2L', default='0L,1L,2L')     
    parser.add_argument('-p', '--periodMC', help='MC period e.g. a,d,e', default='a,d,e')     
    parser.add_argument('-d', '--debug', action='store_true',help='debugging mode')     

    args = parser.parse_args()     
    print parser.parse_args()

    stemlist=args.stemlist
    list_channel=args.channels.split(",")
    list_mcType=args.periodMC.split(",")
    debug=args.debug
    if debug:
        print "stemlist",stemlist
        print "list_channel",list_channel
        print "list_mcType",list_mcType

    # map channel to derivation for input file name
    deriv_dict={
        '0L':'HIGG5D1',
        '1L':'HIGG5D2',
        '2L':'HIGG2D4'
        }    

    for channel in list_channel:
        for mcType in list_mcType:
            deriv=deriv_dict.get(channel)
            if debug:
                print 'channel',channel
                print 'derivation',deriv
            infile=stemlist+"_"+mcType+"."+deriv+".txt"
            outfile="dslist_NevtDxAOD_"+deriv+"_"+mcType+".txt"
            logfile="run_getAMIInfo_"+channel+"_"+mcType+".out"
            if debug:
                print 'infile',infile
                print 'outfile',outfile
                print 'logfile',logfile
            command="nohup python ../../../scripts/getAMIInfo.py "+infile+" -o "+outfile+" >& "+logfile+" &"
            print "command="+command
            os.system(command)

    print ""
    print ""
    print "Finished launching runAllGetAMIInfo"
