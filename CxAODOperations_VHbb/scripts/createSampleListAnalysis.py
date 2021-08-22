#!/usr/bin/env python   
# Created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis
# to create the sample lists to run give that we run d, and if not present c, and they have two AMI tag for one DSID
# and also we may want to use a subset for a given analysis, say VHbb, for faster processing

# read command line arguments
import os,sys
# run and process output of bash commands
import subprocess

total = len(sys.argv)
# number of arguments plus 1
if total!=1:
    print "You need some arguments, will ABORT!"
    print "Ex: ",sys.argv[0]," "
    assert(False)
# done if

#################################################################
################### Configurations ##############################
#################################################################

debug=False

analysis="VHbb"
list_derivation="HIGG5D1,HIGG5D2,HIGG2D4".split(",")
list_mcType="a,d".split(",")

#################################################################
################### Functions ###################################
#################################################################

def get_list_of_used_DSID(inputSampleUsed):
    if debug:
        print " "
        print "get list from inputSampleUsed",inputSampleUsed
    list_DSID=[]
    with open(inputSampleUsed,'r') as inputFile:
        for line in inputFile:
            line=line.rstrip()
            DSID=line
            if debug:
                print "DSID",DSID
            list_DSID.append(DSID)
    if debug:
        print "list_DSID",list_DSID
    return list_DSID
# done function

def create_sample_list_used_analysis(derivation,mcType):
    if debug:
        print "Start create_sample_list_used_analysis(derivation,mcType)"
        print "derivation",derivation,"mcType",mcType
    #
    inputSampleFileName ="list_sample_grid.13TeV_25ns.mcdata_"+mcType+"."+derivation+".txt"
    outputSampleFileName ="list_sample_grid.13TeV_25ns.mcdata"+analysis+"_"+mcType+"."+derivation+".txt"
    analysisDSIDFileName="../../data/sample_use_"+derivation+"_"+analysis+".txt"
    if debug:
        print "inputSampleFileName",inputSampleFileName
        print "outputSampleFileName",outputSampleFileName
        print "analysisDSIDFileName",analysisDSIDFileName
    # 
    list_DSID=get_list_of_used_DSID(analysisDSIDFileName)
    # 
    list_dataset=[]
    with open(inputSampleFileName,'r') as inputFile:
        for line in inputFile:
            dataset=line.rstrip()
            if dataset=="":
                continue
            DSID=dataset.split(".")[1]
            if debug:
                print "DSID",DSID
            used=False
            if DSID[0:2]=="00":
                used=True # as data
            if DSID in list_DSID:
                used = True # as MC
            if used:
                list_dataset.append(dataset)
    # 
    outputFile=open(outputSampleFileName,'w')
    for dataset in list_dataset:
        outputFile.write(dataset+"\n")
    outputFile.close()
# done function

def doIt():
    for derivation in list_derivation:
        for mcType in list_mcType:
            create_sample_list_used_analysis(derivation,mcType)
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
