#!/usr/bin/env python
# Created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis
# to compare the number of events in mc16a, mc16c, mc16d, and their ratio
# in a format that is easy to read for both humans and machines

# read command line arguments
import os,sys

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

list_derivation="HIGG5D1,HIGG5D2,HIGG2D4".split(",")
list_mcType=["mc16a","mc16c","mc16d"]

#################################################################
################### Functions##### ##############################
#################################################################

def read_file(derivation):
    dict_DSID_AMITagES_info={}
    inputFileName="summary_mc16_"+derivation+".txt"
    with open(inputFileName,'r') as f:
        for line in f:
            line=line.rstrip()
            if "DSID" in line and "Name" in line:
                continue # skip as first line
            list_lineElement=line.split()
            # e.g. 
            #   DSID                                                              Name                  AMITagES            AMITagRP_mc16a            AMITagRP_mc16c            AMITagRP_mc16d        mc16a        mc16c        mc16d
            # 345038              PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO         e5590_e5984_s3126         -----------------         r9781_r9778_p3374         -----------------            0       473100            0
            # 345038              PowhegPythia8EvtGen_NNPDF30_AZNLO_ZH125J_Zincl_MINLO               e5590_s3126         r9364_r9315_p3374         -----------------         -----------------       473100            0            0
            assert(len(list_lineElement)==9)
            DSID=list_lineElement[0]
            name=list_lineElement[1]
            AMITagES=list_lineElement[2]
            dict_mcType_info={}
            dict_mcType_info["mc16a"]=[list_lineElement[3],float(list_lineElement[6])]
            dict_mcType_info["mc16c"]=[list_lineElement[4],float(list_lineElement[7])]
            dict_mcType_info["mc16d"]=[list_lineElement[5],float(list_lineElement[8])]
            DSID_AMITagES=DSID+"."+AMITagES
            dict_DSID_AMITagES_info[DSID_AMITagES]=[name,dict_mcType_info]
        # done loop over line
    # close file
    if debug:
        for DSID_AMITagES in sorted(dict_DSID_AMITagES_info.keys()):
            info=dict_DSID_AMITagES_info[DSID_AMITagES]
            print "%-40s %-60s" % (DSID_AMITagES,info[0]), info[1]
    return dict_DSID_AMITagES_info
# done function

# for a given DSID, sum all the AMI tags 
# there are two or even three AMITagESs for extensions
# we want to compare the number of events of total a, c, and d

def ratio(a,b):
    if b==0:
        result=0
    else:
        result=a/b
    return result
# done function

def compare(derivation):
    dict_DSID_AMITagES_info=read_file(derivation)
    dict_DSID_info={}
    for DSID_AMITagES in sorted(dict_DSID_AMITagES_info.keys()):
        DSID=DSID_AMITagES.split(".")[0]
        AMITagES=DSID_AMITagES.split(".")[1]
        info=dict_DSID_AMITagES_info[DSID_AMITagES]
        name=info[0]
        dict_mcType_info_current=info[1]
        if debug:
            print "DSID",DSID,"AMITagES",AMITagES,"dict_mcType_info_current",dict_mcType_info_current
        dict_mcType_nrEvents_current={}
        for mcType in list_mcType:
            dict_mcType_nrEvents_current[mcType]=dict_mcType_info_current[mcType][1]
        if debug:
            print "DSID",DSID,"AMITagES",AMITagES,"dict_mcType_nrEvents_current",dict_mcType_nrEvents_current
        if DSID not in dict_DSID_info.keys():
            if debug:
                print "DSID",DSID,"Not found, so creating it with the info of the first one"
            dict_DSID_info[DSID]=[name,dict_mcType_nrEvents_current]
        else:      
            if debug:
                print "DSID",DSID,"Already present, so adding the current one"
            for mcType in list_mcType:
                dict_DSID_info[DSID][1][mcType]+=dict_mcType_nrEvents_current[mcType]
        if debug:
            print "DSID",DSID,"After this, the sum is"
            print "%-40s %-60s" % (DSID,dict_DSID_info[DSID]), dict_DSID_info[DSID]
    # done loop over first dictionary
    if debug:
        for DSID in sorted(dict_DSID_info.keys()):
            info=dict_DSID_info[DSID]
            print "%-40s %-60s" % (DSID,info[0]), info[1]
    return dict_DSID_info
# done function

def create_file_sum(derivation,dict_DSID_info):
    outputFileName="summary_mc16_sum_"+derivation+".txt"
    outputFile=open(outputFileName,"w")
    text="%6s %65s %12s %12s %12s %10s %10s %10s" % ("DSID","Name","mc16a","mc16c","mc16d", "R_mc16a", "R_mc16c", "R_mc16d")
    outputFile.write(text+"\n")
    for DSID in sorted(dict_DSID_info.keys()):
        info=dict_DSID_info[DSID]
        name=info[0]
        text="%6s %65s" % (DSID,name)
        for mcType in list_mcType:
            text+=" %12.0f" % (info[1][mcType])
        for mcType in list_mcType:
            myRatio=ratio(info[1][mcType],info[1]["mc16a"])
            text+=" %10.2f" % (myRatio)
        outputFile.write(text+"\n")
    outputFile.close()
# done function

def create_file_sample_used(derivation,dict_DSID_info):
    # create the sample list to use
    # if a is present, add it to a
    # if some d is present, add all the AMI tags of that sample to d
    # if no d is present, add all that sample to c
    if debug:
        print ""
        print ""
        print "Start create_file_sample_used(derivation,dict_DSID_AMITagES_info,dict_DSID_info)"
    dict_DSID_AMITagES_info=read_file(derivation)
    for mcType in list_mcType:
        if debug:
            print "mcType",mcType
        outputFile=open("list_sample_grid."+mcType+"_used."+derivation+".txt","w")
        for DSID_AMITagES in sorted(dict_DSID_AMITagES_info.keys()):
            DSID=DSID_AMITagES.split(".")[0]
            AMITagES=DSID_AMITagES.split(".")[1]
            info=dict_DSID_AMITagES_info[DSID_AMITagES]
            name=info[0]
            dict_mcType_info_current=info[1]
            if doTestSomeDSID==True:
                if DSID not in ["342282","410470","364147"]:
                    continue
            if debug:
                print "DSID",DSID,"AMITagES","name",name,AMITagES,"dict_mcType_info_current",dict_mcType_info_current
            #continue
            if debug:
                print "dict_DSID_info[DSID]",dict_DSID_info[DSID]
                print "dict_DSID_info[DSID][1]",dict_DSID_info[DSID][1]
            # check if for this DSID the sum is zero or not
            nrEventsSum=dict_DSID_info[DSID][1][mcType]
            if debug:
                print "nrEventsSum",nrEventsSum
            if mcType=="mc16a" or mcType=="mc16d":
                skipDSID=(nrEventsSum==0)
            elif mcType=="mc16c":
                # if mc16d is not present, then try to use mc16c
                skipDSID=(dict_DSID_info[DSID][1]["mc16d"]!=0 or nrEventsSum==0)
            if debug:
                print "skipDSID",skipDSID
            if skipDSID==True:
                continue
            # check the nr of events for this particular AMITagES is not zero
            AMITagRP=dict_mcType_info_current[mcType][0]
            nrEvents=dict_mcType_info_current[mcType][1]
            if debug:
                print "AMITagRP",AMITagRP,"nrEvents",nrEvents
            if nrEvents==0:
                continue
            # this pair of DSID and AMITagES will be written
            # recreate dataset name
            # e.g.. mc16_13TeV.304014.MadGraphPythia8EvtGen_A14NNPDF23_3top_SM.deriv.DAOD_HIGG5D1.e4324_s3126_r9364_r9315_p3371
            dataset="mc16_13TeV."+DSID+"."+name+".deriv.DAOD_"+derivation+"."+AMITagES+"_"+AMITagRP
            if debug:
                print "dataset",dataset
            text=dataset
            outputFile.write(text+"\n")
        outputFile.close()
    # done loop over mcType
# done function

def doIt():
    for derivation in list_derivation:
        dict_DSID_info=compare(derivation)
        create_file_sum(derivation,dict_DSID_info)
        create_file_sample_used(derivation,dict_DSID_info)
    # done loop over derivation
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
print "All finished fine."
