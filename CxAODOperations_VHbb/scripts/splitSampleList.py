#!/usr/bin/env python
# Created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis
# to split the sample lists into alternative and nominal
# alternative: data + MC alternative, run without systematics, so faster, use more inputs per job
# nominal: MC nominal, run with systematics, so slower, use fewer inputs per job

# read command line arguments
import sys
import os

total = len(sys.argv)
# number of arguments plus 1
if total!=4:
    print "You need one. I will ABORT!"
    print "Ex: ",sys.argv[0]," stems        sampleType derivations"
    print "Ex: ",sys.argv[0]," mcdata,test  a,c,d      HIGG5D1,HIGG5D2,HIGG2D4"
    print "Ex: ",sys.argv[0]," mcdata       a          HIGG2D4"
    sys.exit(1)
# done if 

#################################################################
################### Configurations ##############################
#################################################################

debug   = False
verbose = True

list_stems=sys.argv[1].split(",")
list_sampleType=sys.argv[2].split(",")
list_derivation=sys.argv[3].split(",")
if debug:
    print "list_stems",list_stems
    print "list_sampleType",list_sampleType
    print "list_derivation",list_derivation

#################################################################
################### Functions##### ##############################
#################################################################

def get_dict_DSID_sampleInfo():
    # splits the samples into nominal and alternative samples
    # based on information that is defined in the sample_info.txt
    # return two lists, nominal and alternative

    IDX_NOMALT = 3  # column 3 in sample_info.txt specifies nominal / alternative

    fwsub = os.path.join(os.getenv('WorkDir_DIR'), '..', '..', 'source','FrameworkSub')
    if not os.path.isdir(fwsub):
        print('Error: Could not find FrameworkSub here: {}'.format(fwsub))
        return None
    # find the list with the target samplenames                                                                                                                                                                                                  
    sample_info_file = os.path.join(fwsub, 'data', 'sample_info.txt')
    if not os.path.isfile(sample_info_file):
        print('Error: Could not find sample info file here: {}'
              .format(sample_info_file))
        return None
    # read in nominal/alternative column from sample_info.txt
    dict_DSID_sampleInfo = {}
    with open(sample_info_file) as sif:
        for line in sif:
            if not line or line.isspace():
                continue
            if line.startswith('#'):
                continue
            split =  line.split()
            try:
                dsid = int(split[0])
            except KeyError:
                continue
            except TypeError:
                continue
            dict_DSID_sampleInfo[dsid] = split[IDX_NOMALT]
    # done read sample info file
    if debug:
        print "dict_DSID_sampleInfo"
        print dict_DSID_sampleInfo
    return dict_DSID_sampleInfo
# done function

def writeSample(dict_split_file,split,sample,commented):
    if commented==True:
        text="# "+sample
    else:
        text=sample
    dict_split_file[split].write(text+"\n")
# done function

def doIt(dict_DSID_sampleInfo,stem,sampleType,derivation):
    # original sample file and check it exists
    sample_list_file = os.path.join('./', 'list_sample_grid.13TeV_25ns.'+stem+'_'+sampleType+'.'+derivation+'.txt')
    if not os.path.isfile(sample_list_file):
        print('Error: Could not find sample list file here: {}'
              .format(sample_list_file))
        return None
    # read the file
    list_split="nominal,alternative".split(",")
    dict_split_file={}
    for split in list_split:
        dict_split_file[split]=open('list_sample_grid.13TeV_25ns.'+stem+'_'+split+'_'+sampleType+'.'+derivation+'.txt','w')
    with open(sample_list_file) as f:
        for line in f.readlines():
            sample=line.rstrip()
            if debug:
                print "sample",sample
            commented=False
            if sample.startswith("# "):
                sample=sample.replace("# ","")
                commented=True
            if debug:
                print "sample",sample,"commented",commented
            # if data sample, write to alternative and continue
            if sample.startswith("data"):
                split="alternative"
                writeSample(dict_split_file,split,sample,commented)
                continue
            # once here, it can only be MC
            try:
                dsid = int(sample.split('.')[1])
            except:
                print("Error: Cannot read DSID from sample '{}'".format(sample))
                raise
            if debug:
                print "dsid",dsid,type(dsid)
            if not dsid in dict_DSID_sampleInfo:
                print('Warning: Cannot find sample_info so putting into nominal (wity systematics) for sample commented={} \n{}'.format(commented,sample))
                split="nominal"
                writeSample(dict_split_file,split,sample,commented)
                continue
            # once here, it can only be MC found in sample_info.txt
            split=dict_DSID_sampleInfo[dsid]
            assert(split=="nominal" or split=="alternative")
            writeSample(dict_split_file,split,sample,commented)
        # done for loop over samples
    # close input file
# done function

def doItAll():
    dict_DSID_sampleInfo=get_dict_DSID_sampleInfo()
    for stem in list_stems:
        for sampleType in list_sampleType:
            for derivation in list_derivation:
                if debug or verbose:
                    print "stem",stem,"sampleType",sampleType,"derivation",derivation
                    doIt(dict_DSID_sampleInfo,stem,sampleType,derivation)

# done function

#################################################################
################### Run #########################################
#################################################################

if __name__ == '__main__':
    doItAll()

#################################################################
################### Finished ####################################
#################################################################
