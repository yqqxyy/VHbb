#!/usr/bin/env python 
#
# check percentage of available DAOD events that were used in a production by comparing DxAOD yields (from CxAOD files)
# with the AMI DxAOD yields 
# Note on yields files format:
# yields file where 3rd column is number of DxAOD input events made using count_Nentry_SumOfWeight.py and countNumberInDxAOD=True
#
from ConfigParser import ConfigParser
from ConfigParser import Error as ConfigError
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser()     
    parser.add_argument('-y', '--yieldsfile', help='CxAOD DAOD yields file', default='dslist_NevtDxAOD_HIGG5D2_e.txt')     
    parser.add_argument('-a', '--amifile', help='AMI DxAOD yields', default='dslist_NevtDxAOD_HIGG5D2_e.txt')     
    parser.add_argument('-o', '--outputfile', help='output file', default='dslist_NevtDxAOD_CxAODfraction.txt')     
    parser.add_argument('-d', '--debug', action='store_true',help='debugging mode')     

    args = parser.parse_args()     
    print parser.parse_args()

    # inputs
    yields_file_name=args.yieldsfile
    in_file_name =args.amifile
    out_file_name = args.outputfile
    #folder_out=args.outputFolder
    debug=args.debug


    print "Input file (DxAOD yields from CxAODs):", yields_file_name
    print "Input file (DxAOD events from AMI):", in_file_name
    print "Running ..."

    try:
        yields_file = open(yields_file_name, 'r')
    except IOError:
        print 'could not open', yields_file_name
        
    try:
        in_file = open(in_file_name, 'r')
    except IOError:
        print 'could not open', in_file_name

    out_file = open(out_file_name, 'w')

    # loop over lines from yield file made to compare with AMI and fill yields array
    yields_arr=[]
    if debug:
        print "Loop over lines from yieldsfile"
    for yields in yields_file :
        p = yields.split()
        if debug:
            print "list",p
        if len(p)!=5:
            print "row",p,"from yield files doesn't have 5 elements. Will ABORT!!!"
            assert(False)
        # datasetid, nDxAOD, AMItags
        yields_arr.append([p[0],p[2],p[4]])
    # done loop over lines in yields files for comparing with AMI

    # loop over lines from file with AMI info and fill DAOD array
    daod_arr=[]
    if debug:
        print "Loop over lines from AMI file"
    for lines in in_file:
        p = lines.split()
        if debug:
            print "list",p
        if len(p)!=4:
            print "row",p,"from file of AMI info does not have 4 elements. Will ABORT!!!"
            assert(False)
        # dsid, nDAOD, AMITag, name
        daod_arr.append([p[0],p[1],p[2],p[3]])
    # done loop over lines from file with AMI info

    if debug:
        print "Loop over yields:"
        for yields in yields_arr:
            print "yields",yields
        print "Loop over daod:"
        for daod in daod_arr:
            print "daod",daod
        print ""

# loop over the AMI values and see which fraction you have in the derivation
# but if you have others that are not in AMI it will be flagged below
    nomatch_arr=[] # to avoid listing twice
    for daod in daod_arr:
        #  print '', daod[0]
        match=False
        for yields in yields_arr:
            if debug:
                print "compare",yields[0],"with",daod[0],"and",yields[2],"with",daod[2],"value",yields[1],"with",daod[1]
            matchDSID   =(yields[0]==daod[0])
            matchAMITags=(yields[2]==daod[2])
            if matchDSID and matchAMITags:
                match=True
                if match and debug:
                    print 'match',yields[0],"with",daod[0],"and",yields[2],"with",daod[2],"value",yields[1],"with",daod[1]
                break
            # done if
        # done loop over line in yield files
        if not match:
            # add to the list of not found
            if daod[0] not in nomatch_arr: 
                nomatch_arr.append(daod[0])
                print 'No match found for dataset ', daod[0], daod[2]
            # create a dummy value of yields with zero yield
            yields=[daod[0],0.0,daod[2]]
        # done if
        # compute the current and total percent
        rat=float(yields[1])/float(daod[1]) 
        if debug:
            print "yield",yields[1],"ami",daod[1],"ratio",rat
        line="%-10s %-8.3f %-60s %-60s" % (yields[0], rat, yields[2], daod[3])
        out_file.write(line+"\n") 
    # done loop over the AMI info file
    
    print ""
    print "None AMI :"
    # if you have others that are not in AMI it will be studied here
    # loop over the CxAOD yields
    for yields in yields_arr:
        match=False
        for daod in daod_arr:
            if debug:
                print "compare",yields[0],"with",daod[0],"and",yields[2],"with",daod[2],"value",yields[1],"with",daod[1]
            matchDSID   =(yields[0]==daod[0])
            matchAMITags=(yields[2]==daod[2])
            if matchDSID and matchAMITags:
                match=True
                if match and debug:
                    print 'match',yields[0],"with",daod[0],"and",yields[2],"with",daod[2],"value",yields[1],"with",daod[1]
                break
            # done if
        # done loop over line in yield files
        if not match:
            print 'No match found for dataset ', yields[0], yields[2]
    
    
    # 
    yields_file.close()
    out_file.close() 
    in_file.close()
    
    print ""
    print ""
    print "Done all in checkForExtensions.py"
    
    
    
    
    
    
    
    
    
    
    
    
    
