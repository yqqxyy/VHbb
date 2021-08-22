#!/usr/bin/env python
#
# Script which compares the cross section, filter. k-factor for the Framework cross section file with the PMG file
# This throws up lots of warnings for signal files so we allow to veto on those in the comparison either  
# using the DSID or matching a string pattern
#
import os

def match_any_substrings_in_string(list_substring,string,debug=False):
    if debug:
        print "Start match_any_substrings_in_string()"
    match=False
    for substring in list_substring:
        match=(substring in string)
        if (match):
            break
    if debug:
        print "string",string,"list_substring",list_substring,"match",match
    return match
# done match_any_substrings_in_string  function

# input files
workdir = os.getenv('WorkDir_DIR')
print 'workdir',workdir
cxaod_filename=workdir+"/../../source/CxAODOperations/data/XSections_13TeV.txt"
print 'cxaod_filename',cxaod_filename
pmg_filename="/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/PMGTools/PMGxsecDB_mc16.txt"
print 'pmg_filename',pmg_filename

debug=False

# veto signals based on DSID or pattern (see below)
veto_list=[
    # SMVH
    '341094','341095','341096','341097','341098','341099',
    '341100','341101','341102','341178','341179','341181','341182',
    '342616','342617','342618',
    '345038','345039','345040',
    '345053','345054','345055','345056','345057','345058',
    '345097','345109','345110','345111','345112','345113','345114',
    # ttH
    '345873','345874','345875',
    # DBL
    '301243','301247','301216','301221','301323','301325','301329','302719','306136','301826','308488','302482','307018'
]

print veto_list

# general patterns to veto
# includes other signals and old Sherpa samples
veto_names =['A14NNPDF23LO_HVT','H125_mum',
             'Sherpa_NNPDF30NNLO_bAbb',
             'A14NNPDF23LO_ggA',
             'A14NNPDF23LO_bbA',
             'CT10_AZNLOCTEQ6L1_ggH',
             'CT10_AZNLOCTEQ6L1_VBF',
             'A14NNPDF23LO_HZZllq',
             'A14NNPDF30LO_A14NNPDF23LO_dmV',
             '14NNPDF23LO_ZZxx',
             'A14NNPDF23LO_WWxx',
             '_hh_bbtt_',
             '_vxjjn1n1',
             '_hdn1n1zpjj',
             '_Hmumu','H125_mue','H125_ee','_Hee',
             'Sherpa_CT10_Zee','Sherpa_CT10_Zmumu','Sherpa_CT10_Ztautau','Sherpa_CT10_Znunu','Sherpa_CT10_Wenu_','Sherpa_CT10_Wmunu_','Sherpa_CT10_Wtaunu_',
             'Sherpa_NNPDF30NNLO_Zmumu_Pt','Sherpa_NNPDF30NNLO_Zee_Pt','Sherpa_NNPDF30NNLO_Ztautau_Pt','Sherpa_NNPDF30NNLO_Znunu_Pt',
             'Sherpa_NNPDF30NNLO_Wtaunu_Pt','Sherpa_NNPDF30NNLO_Wenu_Pt','Sherpa_NNPDF30NNLO_Wmunu_Pt',
             'Sherpa_CT10_SinglePhotonPt',
             'AlpgenPythiaEvtGe'
]
print veto_names


# Open CxAOD file and fill info 
try:
    cxaod_file = open(cxaod_filename, 'r')
except IOError:
    print 'could not open', cxaod_filename

# fill list as PMG, dsid, name, xs, kfac, filter eff    
cxaod_list=[]   
for field in cxaod_file :
    p = field.split()
    if len(p) ==6 and p[0].isdigit():
        if p[0] not in veto_list:
            found=match_any_substrings_in_string(veto_names,p[5],debug)
            if not found:
                print "list",p,type(p[0])                
                cxaod_list.append([p[0],p[5],p[1],p[3],p[2]])
        
print cxaod_list

# PMG list
try:
    pmg_file = open(pmg_filename, 'r')
except IOError:
    print 'could not open', pmg_filename

pmg_list=[]   
for field in pmg_file :
    p = field.split()
    if len(p) ==8 and p[0].isdigit():
        pmg_list.append(p)

# loop over CxAOD list and compare with PMG
for cxaod in cxaod_list:
    if debug:
        print '', cxaod[0]
    match=False
    for pmg in pmg_list:
        matchDSID  = (pmg[0]==cxaod[0])
        if matchDSID:
            if debug:
                print 'match',pmg[0],"with",cxaod[0]
                
            # cross sections
            rat=float(pmg[2])/float(cxaod[2]) 
            if debug:
                print "cross sec: pmg",pmg[2],"cxaod",cxaod[2],"ratio",rat
            if abs(1.0-rat)>0.01:
                print 'cross sec problem',abs(1-rat),pmg[0],cxaod[0],pmg[1],cxaod[1]
                print "cross sec problem: pmg",pmg[2],"cxaod",cxaod[2],"ratio",rat
            # filter efficiency
            rat=float(pmg[3])/float(cxaod[3]) 
            if debug:
                print "filteff:pmg",pmg[3],"cxaod",cxaod[3],"ratio",rat
            if abs(1.0-rat)>0.01:
                print 'filt eff problem',abs(1-rat),pmg[0],cxaod[0],pmg[1],cxaod[1]
                print "filteff problem:pmg",pmg[3],"cxaod",cxaod[3],"ratio",rat
            # kfactor
            rat=float(pmg[4])/float(cxaod[4]) 
            if debug:
                print "kfac: pmg",pmg[4],"cxaod",cxaod[4],"ratio",rat
            if abs(1.0-rat)>0.01:
                print 'kfac problem',abs(1-rat),pmg[0],cxaod[0],pmg[1],cxaod[1]
                print 'kfac problem: pmg',pmg[4],"cxaod",cxaod[4],"ratio",rat
