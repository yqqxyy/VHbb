#!/usr/bin/env python
# Created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis
# to create a text file that stores in an easy to read format (both by humans and machines)
# the info of number of events of each derivation samples in mc16a, mc16c, mc16d

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
doTestSomeDSID=False
doTestIgnoreAMI=False

DerivationsIn="VHbb" # VHbb or monoHbb

inputFolderStem ="../.."
inputFolder     =inputFolderStem+"/In/"+DerivationsIn
inputSampleInfo=inputFolderStem+"/data/sample_info.txt"

list_derivation="HIGG5D1,HIGG5D2,HIGG2D4".split(",")
list_mcType="mc16a,mc16c,mc16d".split(",")

#################################################################
################### Functions##### ##############################
#################################################################

def get_list_sample_generic(mcType,derivation):
    return inputFolder+"/list_sample_grid."+mcType+"_13TeV_25ns_*_nominal."+derivation+".txt"
# done function

def get_list_sample_all(mcType,derivation):
    return inputFolder+"/list_sample_grid."+mcType+"_13TeV_25ns_all_nominal."+derivation+".txt"
# done function

def rm_list_sample_all(mcType,derivation):
    command="rm "+inputFolder+"/list_sample_grid."+mcType+"_13TeV_25ns_all_nominal."+derivation+".txt"
    if debug:
        print "command",command
    os.system(command)
# done function

def concatenate_list_sample_mc(listSampleGeneric,listSampleAll):
    command="cat "+listSampleGeneric+" > "+listSampleAll
    if debug:
        print "command",command
    os.system(command)
# done function

def get_AMITagESA_AMITagRP(AMITag,debug=False):
    # keep only the e and s (FullSim) or a (AtlasFast2) tags, as these is what we need to compare 
    # to see if we have the same sample, or an extension
    list_AMITag=AMITag.split("_")
    counter=0
    for AMITagElement in list_AMITag:
        if AMITagElement[0]=="r" or AMITagElement[0]=="p":
            continue
        # ESA can be e, s (FullSim), but also a (AtlasFast2)
        counter+=1
        if counter==1:
            AMITagESA=AMITagElement
        else:
            AMITagESA+="_"+AMITagElement
    # done loop over AMITagElement
    AMITagRP=AMITag.replace(AMITagESA+"_","")
    if debug:
        print "%-40s %-20s %-20s" % (AMITag, AMITagESA, AMITagRP)
    return AMITagESA,AMITagRP
# done function

def get_dict_DSID_AMITagESA_info(listSampleAll):
    if debug:
        print ""
        print "Start get_dict_DSID_AMITagESA_info for listSampleAll",listSampleAll
    dict_DSID_AMITagESA_info={}
    with open(listSampleAll,'r') as inputFile:
        for dataset in inputFile:
            dataset=dataset.rstrip()
            if dataset=="":
                continue
            if dataset[0]=="#":
                continue
            if debug:
                print "dataset",dataset
            list_datasetElement=dataset.split(".")
            if debug:
                print "list_datasetElement",list_datasetElement
            DSID=list_datasetElement[1]
            name=list_datasetElement[2]
            prodType=list_datasetElement[3] # deriv
            derivation=list_datasetElement[4] # DAOD_HIGG5D1
            AMITag=list_datasetElement[5] # e5626_e5984_s3126_r10201_r10210_p3371
            AMITagESA,AMITagRP=get_AMITagESA_AMITagRP(AMITag,debug=debug) # keep only e and s or s tags; then only r and p tags
            dict_DSID_AMITagESA_info[DSID+"."+AMITagESA]=[name,dataset,AMITagRP]
    # done read file
    if debug:
        for DSID_AMITagESA in sorted(dict_DSID_AMITagESA_info.keys()):
            print "%-55s %-70s" % (DSID_AMITagESA,dict_DSID_AMITagESA_info[DSID_AMITagESA][0])
    return dict_DSID_AMITagESA_info
# done function

def get_AMIInfo(dataset,debug=False):
    if debug:
        print "dataset",dataset
    # get the AMI info
    try:
        pycmd = subprocess.Popen(['ami', 'show', 'dataset', 'info', dataset],
                                  stdout=subprocess.PIPE)
    except OSError:
        print('Error: ami is not set up')
        print('Did you forget to lsetup PyAMI?')
        sys.exit(1)
    dict_var_value = {}
    for line in pycmd.stdout:
        line = line.rstrip()
        if debug:
            print "line",line
        list_line = line.split(":")
        dict_var_value[list_line[0].replace(" ","")] = list_line[1].replace(" ","")
    # done loop over lines from ami output
    if debug:
        for var in dict_var_value:
            print "var",var,"value",dict_var_value[var]
    # dataset is good if it has at least some events
    # we may add more requirements in the future
    if "totalEvents" not in dict_var_value.keys():
        print "WARNING!!! For dataset",dataset,"from AMIInfo totalEvents not found. Will set to 0.0"
        #assert(False)
        nrEvents=0.0
    else:
        nrEvents=float(dict_var_value["totalEvents"])
    # done if
    if debug:
        print "nrEvents",nrEvents,"dataset",dataset
    return nrEvents
# done function

def createSummaryFile(dict_mcType_dict_DSID_AMITagESA_info,derivation):
    if debug:
        print ""
        print "Start createSummaryFile"
    dict_DSID_AMITagESA_infoNew={}
    for mcType in list_mcType:
        if debug:
            print "mcType",mcType
        dict_DSID_AMITagESA_info=dict_mcType_dict_DSID_AMITagESA_info[mcType]
        for DSID_AMITagESA in sorted(dict_DSID_AMITagESA_info.keys()):
            list_DSID_AMITagESA=DSID_AMITagESA.split(".")
            DSID=list_DSID_AMITagESA[0]
            AMITag=list_DSID_AMITagESA[1]
            if doTestSomeDSID:
                if not DSID in ["410470", "364193","364194"]:
                    continue
            if debug:
                print "ADRIAN DSID_AMITagESA",DSID_AMITagESA
            info=dict_mcType_dict_DSID_AMITagESA_info[mcType][DSID_AMITagESA]
            if debug:
                print "info",info
            name=info[0]
            dataset=info[1] # this is the same as the derivation dataset
            AMITagFull=info[2] # this is the full AMITag
            if doTestIgnoreAMI:
                nrEvents=99999999
            else:
                nrEvents=get_AMIInfo(dataset,False) 
            if DSID_AMITagESA not in dict_DSID_AMITagESA_infoNew.keys():
                dict_DSID_AMITagESA_infoNew[DSID_AMITagESA]=[name,{mcType:[nrEvents,AMITagFull]}]
            else:
                dict_DSID_AMITagESA_infoNew[DSID_AMITagESA][1][mcType]=[nrEvents,AMITagFull]
        # done loop over DSID_AMITagESA
    # done loop over mcType
    if debug:
        print ""
        print "Looping over the summary info:"
        for DSID_AMITagESA in sorted(dict_DSID_AMITagESA_infoNew.keys()):
            infoNew=dict_DSID_AMITagESA_infoNew[DSID_AMITagESA]
            print DSID_AMITagESA, infoNew
    # done for loop
    outputTempNewFile=open("summary_mc16_"+derivation+".txt","w")
    text="%6s %65s %25s %25s %25s %25s %12s %12s %12s" % ("DSID","Name","AMITagESA","AMITagRP_mc16a","AMITagRP_mc16c","AMITagRP_mc16d","mc16a","mc16c","mc16d")
    outputTempNewFile.write(text+"\n")
    for DSID_AMITagESA in sorted(dict_DSID_AMITagESA_infoNew.keys()):
        infoNew=dict_DSID_AMITagESA_infoNew[DSID_AMITagESA]
        name=infoNew[0]
        dict_mcType_info=infoNew[1]
        list_DSID_AMITagESA=DSID_AMITagESA.split(".")
        DSID=list_DSID_AMITagESA[0]
        AMITagESA=list_DSID_AMITagESA[1]
        text="%6s %65s %25s" % (DSID,name,AMITagESA)
        for mcType in list_mcType:
            if mcType in dict_mcType_info.keys():
                AMITagRP=dict_mcType_info[mcType][1]
            else:
                AMITagRP="-----------------"
            if debug:
                print "AMITagRP",AMITagRP
            text+=" %25s" % (AMITagRP)
        for mcType in list_mcType:
            if mcType in dict_mcType_info.keys():
                nrEvents=dict_mcType_info[mcType][0]
            else:
                nrEvents=0.0
            if debug:
                print "nrEvents",nrEvents
            text+=" %12.0f" % (nrEvents)
        if debug:
            print text
        outputTempNewFile.write(text+"\n")
    # we keep all the samples that exist and have the DSID we want
    # done loop over DSID_AMITagESA
    outputTempNewFile.close()
# done function

def doIt():
    for derivation in list_derivation:
        # concatenate in all sample
        dict_mcType_dict_DSID_AMITagESA_info={}
        for mcType in list_mcType:
            listSampleGeneric=get_list_sample_generic(mcType,derivation)
            listSampleAll    =get_list_sample_all(mcType,derivation)
            concatenate_list_sample_mc(listSampleGeneric,listSampleAll)
            dict_DSID_AMITagESA_info=get_dict_DSID_AMITagESA_info(listSampleAll)
            dict_mcType_dict_DSID_AMITagESA_info[mcType]=dict_DSID_AMITagESA_info
        # done loop over mcType
        if debug:
            for mcType in list_mcType:
                print "mcType",mcType
                for DSID_AMITagESA in sorted(dict_mcType_dict_DSID_AMITagESA_info[mcType].keys()):
                    info=dict_mcType_dict_DSID_AMITagESA_info[mcType][DSID_AMITagESA]
                    print "%-55s %-70s %-150s" % (DSID_AMITagESA,info[0],info[1])
        createSummaryFile(dict_mcType_dict_DSID_AMITagESA_info,derivation)
        # at the end remove the all sample lists
        for mcType in list_mcType:
            rm_list_sample_all(mcType,derivation)
        # done loop over mcType
            
# done function

#################################################################
################### Run #########################################
#################################################################

doIt()

#################################################################
################### Finished ####################################
#################################################################
