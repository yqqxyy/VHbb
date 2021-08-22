#!/usr/bin/env python   
# Created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis
# to create the list of PRW file (NTUP_PILEUP) for the samples we decided to use

# read command line arguments
import os,sys
# run and process output of bash commands
import subprocess
# set
from sets import Set
# find list of files based on pattern
import glob

total = len(sys.argv)
# number of arguments plus 1
if total!=4:
    print "You need some arguments, will ABORT!"
    print "Ex: ",sys.argv[0]," stem       list_derivation          list_mcType"
    print "Ex: ",sys.argv[0]," mcdata     HIGG5D1,HIGG5D2,HIGG2D4  a,d"
    print "Ex: ",sys.argv[0]," mcdataVHbb HIGG5D1,HIGG5D2,HIGG2D4  a,d"
    print "Ex: ",sys.argv[0]," test       HIGG2D4                  a"
    assert(False)
# done if

#################################################################
################### Configurations ##############################
#################################################################

debug=False
verbose=True
doTestOneDSID=False
doTestSomeDSID=False

stem=sys.argv[1]
list_derivation=sys.argv[2].split(",") # ["HIGG5D1","HIGG5D2","HIGG2D4"]
list_mcType=sys.argv[3].split(",") # [a,d]

#################################################################
################### Functions ###################################
#################################################################

# from all the derivation samples we create, let's create the unique list of DAOD
# note unique, as three derivations (5D1, 5D2, 2D4) have the same AOD
# we could use the ones of "_used"+derivation, 
# but for mc16c it contains only those not in d
# but to be able to still compare c and d, let's really be inclusive
# so we cat all the samples, in a set, to keep each only once 
def get_list_datasetDAOD(mcType,derivation):
    if debug:
        print "Start get_list_datasetDAOD("+mcType+")"
    # e.g. list_sample_grid.13TeV_25ns.mcdataVHbb_a.HIGG5D1.txt
    list_fileDAOD=glob.glob("list_sample_grid.13TeV_25ns."+stem+"_"+mcType+"."+derivation+".txt")
    set_datasetDAOD=Set()
    for fileDAOD in list_fileDAOD:
        if debug:
            print "fileDAOD",fileDAOD
        with open(fileDAOD,'r') as inputFile:
            for line in inputFile:
                datasetDAOD=line.rstrip()
                if datasetDAOD[0]=="#" or datasetDAOD=="":
                    continue
                if "physics_Main" in datasetDAOD:
                    continue # skip data DxAOD, as PRW is only for MC
                set_datasetDAOD.add(datasetDAOD)
    # done loop over files
    list_datasetDAOD=sorted(list(set_datasetDAOD))
    outputFileName="list_sample_grid.13TeV_25ns.goodMC."+stem+"_"+mcType+"."+derivation+".txt"
    outputFile=open(outputFileName,"w")
    for datasetDAOD in list_datasetDAOD:
        if debug:
            print "datasetDAOD",datasetDAOD
        outputFile.write(datasetDAOD+"\n")
    # done loop over datasetDAOD
    outputFile.close()
    return list_datasetDAOD
# done function

def get_elements_from_dataset(dataset,debug=False):
    if debug:
        print "get_elements_from_dataset("+dataset+"):"
    list_datasetElement=dataset.split(".")
    if debug:
        print "list_datasetElement",list_datasetElement
    mcPrefix=list_datasetElement[0] # mc16_13TeV
    DSID=list_datasetElement[1] # 410503
    name=list_datasetElement[2] # PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_dil
    sampleType1=list_datasetElement[3] # deriv
    sampleType2=list_datasetElement[4] # DAOD_HIGG5D1
    AMITag=list_datasetElement[5] # e5475_e5984_s3126_r10201_r10210_p3371
    if debug:
        print "%-10s %-160s" % ("mcPrefix",mcPrefix)
        print "%-10s %-160s" % ("DSID",DSID)
        print "%-10s %-160s" % ("name",name)
        print "%-10s %-160s" % ("sampleType1",sampleType1)
        print "%-10s %-160s" % ("sampleType2",sampleType2)
        print "%-10s %-160s" % ("AMITag",AMITag)
    return mcPrefix,DSID,name,sampleType1,sampleType2,AMITag
# done function

# get from AMITag the part that defines uniquely the AOD
# all the e-tags, all the s-tags, all the a-tags, and all the r-tags (no p-tags)
def get_AMITagAOD(AMITag,debug=False):
    list_AMITagElement=AMITag.split("_")
    keep=False
    counterKeep=0
    for AMITagElement in list_AMITagElement:
        if debug:
            print "AMITagElement",AMITagElement
        if AMITagElement[0]=="e" or AMITagElement[0]=="s" or AMITagElement[0]=="a" or AMITagElement[0]=="r":
            keep=True
        else:
            keep=False
        # done if
        if keep==False:
            continue
        counterKeep+=1
        if counterKeep==1:
            AMITagAOD=AMITagElement
        else:
            AMITagAOD+="_"+AMITagElement
    # done for loop over elements of AMITag
    if debug:
        print "%30s %20s" % (AMITag, AMITagAOD)
    return AMITagAOD
# done function

# get from AMITag the part that defines uniquely the PRW
# all the e-tags, all the s-tags, all the a-tags, and only the first r-tag
def get_AMITagPRW(AMITag,debug=False):
    list_AMITagElement=AMITag.split("_")
    keep=False
    counterRTag=0
    counterKeep=0
    for AMITagElement in list_AMITagElement:
        if debug:
            print "AMITagElement",AMITagElement
        if AMITagElement[0]=="e" or AMITagElement[0]=="s" or AMITagElement[0]=="a":
            keep=True
        elif AMITagElement[0]=="r":
            counterRTag+=1
            if counterRTag==1:
                keep=True
            else:
                keep=False
        else:
            keep=False
        # done if
        if keep==False:
            continue
        counterKeep+=1
        if counterKeep==1:
            AMITagPRW=AMITagElement
        else:
            AMITagPRW+="_"+AMITagElement
    # done for loop over elements of AMITag
    if debug:
        print "%30s %20s" % (AMITag, AMITagPRW)
    return AMITagPRW
# done function

# e.g. mc16_13TeV.302216.MadGraphPythia8EvtGen_A14NNPDF23LO_HVT_Agv1_VcWZ_llqq_m0500.merge.AOD.e4148_e5984_s3126_r9364_r9315
def get_datasetAOD(mcPrefix,DSID,name,AMITagAOD,debug=False):
    datasetAOD=mcPrefix+"."+DSID+"."+name+".merge.AOD."+AMITagAOD
    if debug:
        print "datasetAOD",datasetAOD
    return datasetAOD
# done function

# e.g. mc16_13TeV.364100.Sherpa_221_NNPDF30NNLO_Zmumu_MAXHTPTV0_70_CVetoBVeto.merge.NTUP_PILEUP.e5271_s3126_r10201*
def get_datasetPRWStem(mcPrefix,DSID,name,AMITagPRW,debug=False):
    #datasetPRWStem=mcPrefix+"."+DSID+"."+name+".merge.NTUP_PILEUP."+AMITagPRW+"_*"
    #datasetPRWStem=mcPrefix+"."+DSID+"."+name+".deriv.NTUP_PILEUP."+AMITagPRW+"_*"
    datasetPRWStem=mcPrefix+"."+DSID+"."+name+".*.NTUP_PILEUP."+AMITagPRW+"_*" # either merge or deriv
    if debug:
        print "datasetPRWStem",datasetPRWStem
    return datasetPRWStem
# done function

def get_nrEvents_from_AMI(dataset):
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
        result = 0
    else:
        result=int(dict_var_value["totalEvents"])
    # done if
    if debug:
        print "nrEvents",result,"dataset",dataset
    return result
# done function

def get_nrEvents_from_RUCIO(dataset):
    if debug:
        print "dataset",dataset
    # get the AMI info
    try:
        pycmd = subprocess.Popen(['rucio', 'list-files', dataset],
                                  stdout=subprocess.PIPE)
    except OSError:
        print('Error: rucio is not set up')
        print('Did you forget to lsetup rucio?')
        sys.exit(1)
    keyString="Total events :"
    nrEvents=0.0
    for line in pycmd.stdout:
        line = line.rstrip()
        if debug:
            print "line",line
        if keyString in line:
            nrEvents=float(line.replace(keyString,""))
    # done loop over lines from rucio output
    if debug:
        print "nrEvents",nrEvents,"dataset",dataset
    return nrEvents
# done function

def ratio(a,b):
    if b==0:
        result=0
    else:
        result=a/b
    return result
# done function

def get_list_datasetAOD(mcType):
    # from the several derivations we reach the same AOD, so use a set
    outputFileName="list_sample_grid.AOD."+mcType+".txt"
    set_datasetAOD=Set()
    # loop over derivation
    for derivation in list_derivation:
        list_datasetDAOD=get_list_datasetDAOD(mcType,derivation)
        # loop over datasetDAOD
        for datasetDAOD in list_datasetDAOD:
            mcPrefix,DSID,name,sampleType1,sampleType2,AMITagDAOD=get_elements_from_dataset(datasetDAOD,debug)
            AMITagAOD=get_AMITagAOD(AMITagDAOD,debug)
            datasetAOD=get_datasetAOD(mcPrefix,DSID,name,AMITagAOD,debug)
            set_datasetAOD.add(datasetAOD)
        # done loop over datasetDAOD
    # done loop over derivations
    list_datasetAOD=sorted(list(set_datasetAOD))
    outputFile=open(outputFileName,"w")
    # loop over datasetDAOD
    for datasetAOD in list_datasetAOD:
        if debug:
            print "datasetAOD",datasetAOD
        outputFile.write(datasetAOD+"\n")
    # done loop over datasetAOD
    outputFile.close()
    # finished
    return list_datasetAOD
# done function

def get_datasetPRW(datasetAOD,summaryFile):
    nrEventsAOD=max(get_nrEvents_from_RUCIO(datasetAOD),get_nrEvents_from_AMI(datasetAOD))
    text="%-6s %-150s %13.0f %5s" % ("AOD",datasetAOD,nrEventsAOD, "---")
    if debug or verbose:
        print text
    summaryFile.write(text+"\n")
    mcPrefix,DSID,name,sampleType1,sampleType2,AMITagAOD=get_elements_from_dataset(datasetAOD,debug)
    AMITagPRW=get_AMITagPRW(AMITagAOD,debug)
    datasetPRWStem=get_datasetPRWStem(mcPrefix,DSID,name,AMITagPRW,debug)
    # create rucio command to find out all the samples, if any, that exist
    command="rucio ls --short "+datasetPRWStem
    if debug:
        print "command",command
    rucio_input=datasetPRWStem
    try:
        proc = subprocess.Popen(['rucio', 'ls', '--short', rucio_input], stdout=subprocess.PIPE)
    except OSError:
        print('rucio is not configured. Cannot continue.')
        print('Did you forget to lsetup rucio?')
        sys.exit(1)
    # get output
    list_datasetPRW=[]
    list_NrEventsRatio=[]
    for line in proc.stdout:
        line=line.rstrip()
        # keep only containers by remoing _tid
        if "_tid" in line:
            continue
        # remove from the front mc16_13TeV:
        datasetPRW=line.split(":")[1]
        #if True:
        #    print "datasetPRW",datasetPRW
        # sometimes there are more than one, but some of them have been deleted (zero events)
        # we have to check with AMI and skip those with zero events
        nrEventsPRW=max(get_nrEvents_from_RUCIO(datasetPRW),get_nrEvents_from_AMI(datasetPRW))
        #if nrEventsPRW==0:
        #    continue
        nrEventsRatio=ratio(float(nrEventsPRW),float(nrEventsAOD))
        text="%-6s %-150s %13.0f %5.2f" % ("PRW",datasetPRW,nrEventsPRW,nrEventsRatio)
        if debug or verbose:
            print text
        summaryFile.write(text+"\n")
        list_datasetPRW.append(datasetPRW)
        list_NrEventsRatio.append(nrEventsRatio)
    # done loop over elements of line output
    if len(list_datasetPRW)==0:
        print "WARNING! No PRW file found for datasetAOD="+datasetAOD
        datasetPRW=""
    elif len(list_datasetPRW)==1:
        datasetPRW=list_datasetPRW[0]
    else:
        print "WARNING! At least two PRW files found for datasetAOD="+datasetAOD+". Choose ratio closest to 1.0!"
        # choose the ratio closest to 1.0
        min_index=-1
        minAbsValueRatioMinusOne=999999.0
        for i,NrEventsRatio in enumerate(list_NrEventsRatio):
            datasetPRWCurrent=list_datasetPRW[i]
            AbsValueRatioMinusOne=abs(NrEventsRatio-1)
            print "datasetPRW",datasetPRW," NrEventsRatio",NrEventsRatio
            if AbsValueRatioMinusOne<minAbsValueRatioMinusOne:
                minAbsValueRatioMinusOne=AbsValueRatioMinusOne
                min_index=i
        # done for loop, found the best index i
        datasetPRW=list_datasetPRW[min_index]
    # done if
    return datasetPRW
# done function

def get_list_datasetPRW(mcType):
    list_datasetAOD=get_list_datasetAOD(mcType)
    # for every AOD we select only one PRW
    outputFileName="list_sample_grid.PRW."+mcType+".txt"
    outputFile=open(outputFileName,"w")
    summaryFileName="summary.AOD-PRW."+mcType+".txt"
    summaryFile=open(summaryFileName,"w")
    # loop over datasetAOD
    counter=0
    for datasetAOD in list_datasetAOD:
        counter+=1
        if doTestOneDSID:
            if counter>1:
                continue
        if doTestSomeDSID:
            if "306979" not in datasetAOD:
                continue
        datasetPRW=get_datasetPRW(datasetAOD,summaryFile)
        if datasetPRW=="":
            continue
        outputFile.write(datasetPRW+"\n")
    # done loop over datasetAOD
    outputFile.close()
    summaryFile.close()
    # done loop over derivation
# done function

def doIt():
    for mcType in list_mcType:
        get_list_datasetPRW(mcType)
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
