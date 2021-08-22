#!/usr/bin/env python
# Created by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis
# to create fast the sample list for data and MC, making sure samples are not empty and for data in GRL
# Extended by Andreas Hoenle (der.andi@cern.ch) to produce separate outputs for DBL signals
# Extended further by Adrian Buzatu to account for different ptags, merge in mcdata

# Note: the script uses rucio to search for all samples satisfying the provided patterns
#          You supply exactly the rtags to be searched
#          e.g. if you supply one rtag this will try to find the 3-tag datasets
#          The exception is if there is a dataset with one rtag and two e-tags, these need to be commented by hand e.g. 410467,410468,410469
#
# Note: the script comments out any sample not listed in sample_info.txt 
#
# Note: the list_notUseful is used to comment out unneeded samples depending on the analysis/channel
#



# read command line arguments
import sys
import os
# run and process output of bash commands
import subprocess
# read from xml
import xml.etree.ElementTree as ET

if not sys.version_info >= (2, 7):
    print('You need at least python version 2.7 (forgot to setup your workspace?)')
    sys.exit(1)

total = len(sys.argv)
# number of arguments plus 1
if total!=5:
    print "You need one. I will ABORT!"
    print "Ex: ",sys.argv[0]," option                channel  mcType  CheckAMIAllEvents"
    print "Ex: ",sys.argv[0]," VHbb                  0L,1L,2L a,d,e   0"
    print "Ex: ",sys.argv[0]," VHbbWrongMETTrigger   0L,1L        e   0"
    print "Ex: ",sys.argv[0]," DBL                   0L,1L,2L a,d,e   0"
    print "Ex: The three p-tags are in order left to right: 1) Data; 2) MC skimmed (bkg); 3) MC unskimmed (sig)"
    print "Ex: Options VHbb for all except 0L_e and 1L_e 21.2.42.0  for full Run-2                              p3640    p3639    p3641"
    print "Ex: Options VHbb for            0L_e and 1L_e 21.2.42.0  with patch for MET trigger for full Run-2   p3718    p3717    p3719"
    print "Ex: Options VHbbWrongMETTrigger 0L_e and 1L_e 21.2.42.0  for full Run-2                              p3640    p3639    p3641"
    print "Ex: Options DBL  for all                      21.2.42.0  for full Run-2                              p3640    p3639    p3641"
    print "Ex: Options DBLPatch for        0L_e and 1L_e 21.2.42.0  with patch for MET trigger for full Run-2   p3718    p3717    p3719"
    print "Ex: Options VHbb or DBL refers to CxAODTag32, see https://indico.cern.ch/event/768950/contributions/3195251/attachments/1745065/2824852/HiggsDAOD20181031.pdf"
    sys.exit(1)
# done if

#################################################################
################### Configurations ##############################
#################################################################

debug   = False
verbose = True
doTestAfterRucio = False
doNotCommentOut = False

option=sys.argv[1]
list_channel=sys.argv[2].split(",")
list_mcType=sys.argv[3].split(",")
checkInAMIAllEventsAreAvailable=bool(int(sys.argv[4]))
if debug:
    print "option",option
    print "list_channel",list_channel
    print "list_mcType",list_mcType
    print "checkInAMIAllEventsAreAvailable",checkInAMIAllEventsAreAvailable

dict_channel_derivation={
    "0L":"HIGG5D1",
    "1L":"HIGG5D2",
    "2L":"HIGG2D4",
}

list_name=[]
for mcType in list_mcType:
    list_name.append("mcdata_"+mcType)

list_derivation=[]
for channel in list_channel:
    list_derivation.append(dict_channel_derivation[channel])

if debug or verbose:
    print "list_name",list_name
    print "list_derivation",list_derivation

# possible that f means the first processing
# and r means reprocessing
# in ICHEP derivations data17 was f
# in new derivations data17 is r
# while in new derivations data18 is f
# e.g. DxAOD container: mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_HIGG2D4.e6337_e5984_s3126_r10724_r10726_p3639
if option=="VHbb":
    dict_name_list_info = {
        "mcdata_a"     : [
            ["data15","r9264"          ,"p3083_p3640"], # data
            ["data16","r9264"          ,"p3083_p3640"], # data
            ["mc16"  ,"r9364"          ,"p3717"],       # bkg, new tag with 3-tags
            #["mc16"  ,"r9364_r9315"    ,"p3639"],       # bkg
            ["mc16"  ,"r9364"    ,"p3639"],       # bkg, 3-tags 
            #["mc16"  ,"r9364_r9315"    ,"p3641"],       # signal
            ["mc16"  ,"r9364"    ,"p3641"],       # signal
          ],
        "mcdata_d"     : [
            ["data17","r*"             ,"p3399_p3640"], # data
            ["mc16"  ,"r10201"         ,"p3717"],       # bkg, new tag with 3-tags
            #["mc16"  ,"r10201_r10210"  ,"p3639"],       # bkg
            ["mc16"  ,"r10201"  ,"p3639"],       # bkg, 3-tags
            #["mc16"  ,"r10201_r10210"  ,"p3641"],       # signal
            ["mc16"  ,"r10201"  ,"p3641"],       # signal
            ],
        "mcdata_e_0L1L": [
            ["data18","f*m*"           ,"p3718"],       # data
            ["mc16"  ,"r10724"         ,"p3717"],       # bkg, 3-tags
            ["mc16"  ,"r10724"         ,"p3726"],       # signal
            ],
        "mcdata_e_2L"  : [
            ["data18","f*_m*"          ,"p3640"],       # data
            #["mc16"  ,"r10724_r10726"  ,"p3639"],       # bkg multi-tag
            ["mc16"  ,"r10724"  ,"p3639"],       # bkg, 3-tags
            #["mc16"  ,"r10724_r10726"  ,"p3641"],       # signal
            ["mc16"  ,"r10724"  ,"p3641"],       # signal
            ],
        }
elif option=="VHbbWrongMETTrigger":
    dict_name_list_info = {
        "mcdata_e"     : [
            ["data18","f*_m*"          ,"p3640"],       # data
            ["mc16"  ,"r10724_r10726"  ,"p3639"],       # bkg
            ["mc16"  ,"r10724_r10726"  ,"p3641"],       # signal
            ],
        }
elif option=="DBL":
    dict_name_list_info = {
        "mcdata_a"     : [
            ["data15","r9264"          ,"p3083_p3640"], # data
            ["data16","r9264"          ,"p3083_p3640"], # data
            ["mc16"  ,"r9364_r9315"    ,"p3639"],       # bkg
            ["mc16"  ,"r9364_r9315"    ,"p3641"],       # signal
            ["mc16"  ,"r9364_r9315"    ,"p3654"],       # signal other p-tag
          ],
        "mcdata_d"     : [
            ["data17","r*"             ,"p3399_p3640"], # data
            ["mc16"  ,"r10201_r10210"  ,"p3639"],       # bkg
            ["mc16"  ,"r10201_r10210"  ,"p3641"],       # signal
            ["mc16"  ,"r10201_r10210"  ,"p3654"],       # signal other p-tag
            ],
        "mcdata_e"     : [
            ["data18","f*_m*"          ,"p3640"],       # data
            ["mc16"  ,"r10724_r10726"  ,"p3639"],       # bkg
            ["mc16"  ,"r10724_r10726"  ,"p3641"],       # signal
            ["mc16"  ,"r10724_r10726"  ,"p3654"],       # signal other p-tag
            ],
        }
elif option=='DBLPatch':
    # for MC un-skimmed (as used for signal), it is not p3719, but p3726. 
    # since the patch only changes the trigger preselection by adding more MET triggers
    # the physics of p3641 and p3726 is the same, but we add that as for some samples
    # the new p-tag has been requested instead
    # it is only for period e, so a and d we remove
    # also the VHbbPatch is run on the new AOD format with one number per letter
    dict_name_list_info = {
        "mcdata_e"     : [
            ["data18","f*m*"             ,"p3718"], # data
            ["mc16"  ,"r10724"           ,"p3717"], # bkg
            ["mc16"  ,"r10724"           ,"p3726"], # signal
            ],
        }
else:
    print "option",option,"not known. Choose VHbb or DBL or VHbbPatch. Will ABORT!!!"
    assert(False)
# done if

# these are not used by either VHbb or DBL, because they are signals from mono-Hbb
# we keep them to know they exist, but commented out so that we do not run on them
list_notUseful = [['zp2hdm_bb_mzp']]
if option=="VHbb" or option=="VHbbWrongMETTrigger":
    # these are not used in VHbb as they are signals from DBL
    list_notUseful.append(['HVT'])
    list_notUseful.append(['radion'])
    list_notUseful.append(['_RS_G_'])
    list_notUseful.append(['PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH'])
    list_notUseful.append(['PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBFH'])
    list_notUseful.append(['_ggA'])
    list_notUseful.append(['_bbA_mA'])
    list_notUseful.append(['_bbA'])
    list_notUseful.append(['aMcAtNloPythia8EvtGen_A14NNPDF23LO_HZZllqq'])
    list_notUseful.append(['aMcAtNloPythia8EvtGen_A14NNPDF23LO_HZZvvqq'])   
    list_notUseful.append(['PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_VBF'])
    list_notUseful.append(['344180.MGPy8EG_PDF4LHC15_A14NNPDF23LO_bbajj_nonRes'])
    #list_notUseful.append(['346213.Sherpa_225_NNPDF30NNLO_bb_MassiveCB_2Bjets_DiPt150']) # now these samples are needed
    # VHbb not interested in ttV
    list_notUseful.append(['410155.aMcAtNloPythia8EvtGen_MEN30NLO_A14N23LO_ttW'])
    list_notUseful.append(['410156.aMcAtNloPythia8EvtGen_MEN30NLO_A14N23LO_ttZnunu'])
    list_notUseful.append(['410157.aMcAtNloPythia8EvtGen_MEN30NLO_A14N23LO_ttZqq'])
    list_notUseful.append(['410218.aMcAtNloPythia8EvtGen_MEN30NLO_A14N23LO_ttee'])
    list_notUseful.append(['410219.aMcAtNloPythia8EvtGen_MEN30NLO_A14N23LO_ttmumu'])
    list_notUseful.append(['410220.aMcAtNloPythia8EvtGen_MEN30NLO_A14N23LO_tttautau'])
    # VHbb no need for EW V+jets or high Mll Z+jets
    list_notUseful.append(['2jets_Min_N_TChannel'])
    list_notUseful.append(['_Mll100'])
    # channel specific - only works if we call script one channel at a time (as we do) else will be excluded for all channels in the list
    if len(list_channel)==1:
        # Znunu for 0 lep only
        if not '0L' in list_channel:
            list_notUseful.append(['Znunu'])
        # remove ttbar dilep and VZll for 0 lepton
        if '0L' in list_channel:
            list_notUseful.append(['_dilepton'])
            list_notUseful.append(['_dil.deriv'])
            list_notUseful.append(['ZbbZll'])
            list_notUseful.append(['ZqqZll'])
            list_notUseful.append(['WqqZll'])
        # remove ttbar METFilt for 1 and 2 lepton 
        if not '0L' in list_channel:
            list_notUseful.append(['345935.PhPy8EG_A14_ttbarMET100_200_hdamp258p75_nonallhad'])
            list_notUseful.append(['407345.PhPy8EG_A14_ttbarMET200_300_hdamp258p75_nonallhad'])
            list_notUseful.append(['407346.PhPy8EG_A14_ttbarMET300_400_hdamp258p75_nonallhad'])
            list_notUseful.append(['407347.PhPy8EG_A14_ttbarMET400_hdamp258p75_nonallhad'])
        # remove ttbar ptW filtered from 0 and 2 lepton
        if not '1L' in list_channel:
            list_notUseful.append(['345951.PhPy8EG_A14_lepWpT100_200_hdamp258p75_nonallhad'])
            list_notUseful.append(['346031.PhPy8EG_A14_lepWpT200_hdamp258p75_nonallhad'])            
        # open questions - ttbar nonallhad needed for all channels?
        if '2L' in list_channel:
            list_notUseful.append(['nonallhad_AK10J160'])
        # No need to keep the EW VBF diboson
            list_notUseful.append(['_EW6'])
        #list_notUseful.append(['MGPy8EvtGen_ZZjj_vvqq_EW6'])
        #list_notUseful.append(['MGPy8EvtGen_WZjj_llqq_EW6'])
        #list_notUseful.append(['MGPy8EvtGen_WWjj_lvqq_EW6'])
        # These are the LQ signal samples - we exclude for now because they produce themselves
        list_notUseful.append(['aMcAtNloPy8EG_A14N30NLO_LQu_'])
        list_notUseful.append(['aMcAtNloPy8EG_A14N30NLO_LQd_'])
    else:
        print "Not excluding channel specific datasets because more than one channel supplied",channel_list
elif option=="DBL" or option=="DBLPatch":
    None
else:
    print "option",option,"not known. Choose VHbb or DBL or VHbbPatch. Will ABORT!!!"
    assert(False)
# done if

if debug or verbose:
    print "dict_name_list_info",dict_name_list_info
    for name in sorted(dict_name_list_info.keys()):
        print "name",name, "list_info:"
        list_info=dict_name_list_info[name]
        for info in list_info:
            print info
# done if
            
# signal needs to be split into signal (for SM), sigDBL (for DBL), and possibly more
signal_exceptions = {
    'sigDBL': [['HVT'], ['AZh'], ['bbA'], ['ggA'], ['RS_G'], ['VBFH','NW'], ['aMcAtNloPythia8EvtGen_A14NNPDF23LO_HZZllqq'], ['zp2hdm_bb_mzp']],
    # 'sigDMH': [['zp2hdm_bb_mzp']],
}

#pathGRL = "/afs/cern.ch/user/v/vhbbframework/workarea/data/GRL"
pathGRL = "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/"


dict_sampleType_GRL = {
    "data15": "GoodRunsLists/data15_13TeV/20170619/physics_25ns_21.0.19.xml",
    "data16": "GoodRunsLists/data16_13TeV/20180129/physics_25ns_21.0.19.xml",
    "data17": "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.xml",
    "data18": "GoodRunsLists/data18_13TeV/20190318/physics_25ns_Triggerno17e33prim.xml",
}

#################################################################
################### Functions##### ##############################
#################################################################

def match_all_substrings_in_string(list_substring,string,debug=False):
    if debug:
        print "Start match_all_substrings_in_string()"
    match=True
    for substring in list_substring:
        match=match and (substring in string)
    if debug:
        print "string",string,"list_substring",list_substring,"match",match
    return match
# done function

def doIt(list_name, derivation):
    if debug or verbose:
        print "Start doIt()"
        print "list_name",list_name
        print "derivation",derivation
    for name in list_name:
        if debug or verbose:
            print "************** name",name,"***********"
        if debug or verbose:
            print "DO: name, listFileName = nameFile(name)"
        name, listFileName = nameFile(name)
        if debug or verbose:
            print "DO: rucio_all = rucioFile(name)"
        rucio_all = rucioFile(name)
        if debug or verbose:
            print "DO: commented = commentFileForEmptyContainerOrNotInGRL(name, rucio_all)"
        commented = commentFileForEmptyContainerOrNotInGRL(name, rucio_all)
        if debug or verbose:
            print "DO: vetoed = applyVetosFromSampleInfo(commented)"
        vetoed = applyVetosFromSampleInfo(commented)
        if debug or verbose:
            print "DO: writeToFile(name, listFileName, vetoed)"
        # spot the 2 e-tag datasets    
        
        writeToFile(name, listFileName, vetoed)
    # done loop over names
# done function

def nameFile(name):
    if debug:
        print "Start nameFile()"
        print "name",name
    listFileName = ('list_sample_grid.13TeV_25ns.{name}.{derivation}.txt'
                    .format(name=name,
                            derivation=derivation))
    s = '*** Start listFileName: {} ***'.format(listFileName)
    print('*' * len(s))
    print(s)
    print('*' * len(s))
    # done
    return name, listFileName
# done function

def rucioFileTest(derivation):
    if debug:
        print "Start rucioFileTest():"
    rucio_all=[
        #"data18_13TeV.00359472.physics_Main.deriv.DAOD_HIGG5D2.f964_m2020_p3718",
        #"mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_"+derivation+".e6348_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_"+derivation+".e6348_e5984_s3126_r10201_r10210_p3557",
        #"mc16_13TeV.410659.PhPy8EG_A14_tchan_BW50_lept_antitop.deriv.DAOD_HIGG5D1.e6671_e5984_a875_r10724_r10726_p3639",
        #"mc16_13TeV.410659.PhPy8EG_A14_tchan_BW50_lept_antitop.deriv.DAOD_HIGG5D1.e6671_e5984_s3126_s3136_r10724_r10726_p3639",
        #"data18_13TeV.00358096.physics_Main.deriv.DAOD_HIGG5D1.f966_m2020_p3640",
        #"data18_13TeV.00358096.physics_Main.deriv.DAOD_HIGG5D1.f961_m2015_p3640",
        #"data18_13TeV.00358115.physics_Main.deriv.DAOD_HIGG5D1.f966_m2020_p3640",
        #"data18_13TeV.00358115.physics_Main.deriv.DAOD_HIGG5D1.f961_m2015_p3640",
        #"data18_13TeV.00358175.physics_Main.deriv.DAOD_HIGG5D1.f966_m2020_p3640",
        #"data18_13TeV.00358175.physics_Main.deriv.DAOD_HIGG5D1.f961_m2015_p3640",
        #"mc16_13TeV.310040.MGPy8EG_A14N23LO_GGF_radion_ZZ_vvqq_kl35L3_m0700.deriv.DAOD_HIGG5D2.e6741_e5984_s3126_r9364_r9315_p3641",
        #"data15_13TeV.00280273.physics_Main.deriv.DAOD_HIGG5D1.r9264_p3083_p3372",
        #"data17_13TeV.00325024.physics_Main.deriv.DAOD_"+derivation+".f822_m1799_p3372",
        #"data17_13TeV.00332915.physics_Main.deriv.DAOD_"+derivation+".f854_m1850_p3372",
        #"data17_13TeV.00339758.physics_Main.deriv.DAOD_"+derivation+".f889_m1902_p3402",
        # "data15_13TeV.00276245.physics_Main.deriv.DAOD_"+derivation+".r9264_p3083_p3372",
        # "data15_13TeV.00276262.physics_Main.deriv.DAOD_"+derivation+".r9264_p3083_p3372",
        # "mc16_13TeV.342282.PowhegPythia8EvtGen_CT10_AZNLOCTEQ6L1_ggH125_inc.deriv.DAOD_"+derivation+".e4850_e5984_s3126_r9781_r9778_p3374",
        # "mc16_13TeV.304102.MadGraphPythia8EvtGen_A14NNPDF23LO_zp2hdm_bb_mzp600_mA300.deriv.DAOD_"+derivation+".e4429_e5984_s3126_r9781_r9778_p3374",
        # "mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_"+derivation+".e6337_e5984_a875_r9364_r9315_p3371",
        # "mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_"+derivation+".e6337_e5984_s3126_r9364_r9315_p3371",
        # "mc16_13TeV.410471.PhPy8EG_A14_ttbar_hdamp258p75_allhad.deriv.DAOD_"+derivation+".e6337_e5984_a875_r9364_r9315_p3371",
        # "mc16_13TeV.410471.PhPy8EG_A14_ttbar_hdamp258p75_allhad.deriv.DAOD_"+derivation+".e6337_e5984_s3126_r9364_r9315_p3371",
        # "mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_"+derivation+".e6348_e5984_a875_r9364_r9315_p3371",
        # "mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_"+derivation+".e6348_e5984_s3126_r9364_r9315_p3371",
        # "mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_"+derivation+".e6348_e5984_s3126_r9364_r9315_p3557",
        # "mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_HIGG2D4.e6348_e5984_s3126_r9364_r9315_p3371",
        # "mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_HIGG2D4.e6348_e5984_s3126_r9364_r9315_p3557",
        #
        #"mc16_13TeV.410658.PhPy8EG_A14_tchan_BW50_lept_top.deriv.DAOD_"+derivation+".e6671_e5984_s3126_r10201_r10210_p3571",
        #"mc16_13TeV.410658.PhPy8EG_A14_tchan_BW50_lept_top.deriv.DAOD_"+derivation+".e6671_e5984_s3126_r10201_r10210_p3557",
        #"mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_"+derivation+".e6348_e5984_a875_r10201_r10210_p3371",
        #"mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_"+derivation+".e6348_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410472.PhPy8EG_A14_ttbar_hdamp258p75_dil.deriv.DAOD_"+derivation+".e6348_e5984_s3126_r10201_r10210_p3557",
        #"mc16_13TeV.364177.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CFilterBVeto.deriv.DAOD_"+derivation+".e5340_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.364177.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV140_280_CFilterBVeto.deriv.DAOD_"+derivation+".e5340_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.364158.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_BFilter.deriv.DAOD_"+derivation+".e5340_e5984_s3126_s3136_r10201_r10210_p3371",
        #"mc16_13TeV.364158.Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV0_70_BFilter.deriv.DAOD_"+derivation+".e5340_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410648.PowhegPythia8EvtGen_A14_Wt_DR_dilepton_top.deriv.DAOD_"+derivation+".e6615_e5984_a875_r10201_r10210_p3557",
        #"mc16_13TeV.410648.PowhegPythia8EvtGen_A14_Wt_DR_dilepton_top.deriv.DAOD_"+derivation+".e6615_e5984_s3126_r10201_r10210_p3371",
        # 
        #"mc16_13TeV.410644.PowhegPythia8EvtGen_A14_singletop_schan_lept_top.deriv.DAOD_"+derivation+".e6527_e5984_a875_r10201_r10210_p3371",
        #"mc16_13TeV.410644.PowhegPythia8EvtGen_A14_singletop_schan_lept_top.deriv.DAOD_"+derivation+".e6527_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410645.PowhegPythia8EvtGen_A14_singletop_schan_lept_antitop.deriv.DAOD_"+derivation+".e6527_e5984_a875_r10201_r10210_p3371",
        #"mc16_13TeV.410645.PowhegPythia8EvtGen_A14_singletop_schan_lept_antitop.deriv.DAOD_"+derivation+".e6527_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410645.PowhegPythia8EvtGen_A14_singletop_schan_lept_antitop.deriv.DAOD_"+derivation+".e6527_s3126_r10201_r10210_p3571",
        #"mc16_13TeV.410646.PowhegPythia8EvtGen_A14_Wt_DR_inclusive_top.deriv.DAOD_"+derivation+".e6552_e5984_a875_r10201_r10210_p3371",
        #"mc16_13TeV.410646.PowhegPythia8EvtGen_A14_Wt_DR_inclusive_top.deriv.DAOD_"+derivation+".e6552_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410647.PowhegPythia8EvtGen_A14_Wt_DR_inclusive_antitop.deriv.DAOD_"+derivation+".e6552_e5984_a875_r10201_r10210_p3371",
        #"mc16_13TeV.410647.PowhegPythia8EvtGen_A14_Wt_DR_inclusive_antitop.deriv.DAOD_"+derivation+".e6552_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410648.PowhegPythia8EvtGen_A14_Wt_DR_dilepton_top.deriv.DAOD_"+derivation+".e6615_e5984_a875_r10201_r10210_p3557",
        #"mc16_13TeV.410648.PowhegPythia8EvtGen_A14_Wt_DR_dilepton_top.deriv.DAOD_"+derivation+".e6615_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410649.PowhegPythia8EvtGen_A14_Wt_DR_dilepton_antitop.deriv.DAOD_"+derivation+".e6615_e5984_a875_r10201_r10210_p3557",
        #"mc16_13TeV.410649.PowhegPythia8EvtGen_A14_Wt_DR_dilepton_antitop.deriv.DAOD_"+derivation+".e6615_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410654.PowhegPythia8EvtGen_A14_Wt_DS_inclusive_top.deriv.DAOD_"+derivation+".e6552_e5984_a875_r10201_r10210_p3371",
        #"mc16_13TeV.410654.PowhegPythia8EvtGen_A14_Wt_DS_inclusive_top.deriv.DAOD_"+derivation+".e6552_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410655.PowhegPythia8EvtGen_A14_Wt_DS_inclusive_antitop.deriv.DAOD_"+derivation+".e6552_e5984_a875_r10201_r10210_p3371",
        #"mc16_13TeV.410655.PowhegPythia8EvtGen_A14_Wt_DS_inclusive_antitop.deriv.DAOD_"+derivation+".e6552_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410656.PowhegPythia8EvtGen_A14_Wt_DS_dilepton_top.deriv.DAOD_"+derivation+".e6615_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410657.PowhegPythia8EvtGen_A14_Wt_DS_dilepton_antitop.deriv.DAOD_"+derivation+".e6615_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.410658.PhPy8EG_A14_tchan_BW50_lept_top.deriv.DAOD_"+derivation+".e6671_e5984_s3126_r10201_r10210_p3557",
        #"mc16_13TeV.410659.PhPy8EG_A14_tchan_BW50_lept_antitop.deriv.DAOD_"+derivation+".e6671_e5984_a875_r10201_r10210_p3557",
        #"mc16_13TeV.410659.PhPy8EG_A14_tchan_BW50_lept_antitop.deriv.DAOD_"+derivation+".e6671_e5984_s3126_r10201_r10210_p3557",
        # 
        #"mc16_13TeV.364172.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_BFilter.deriv.DAOD_"+derivation+".e5340_e5984_s3126_r10201_r10210_p3371",
        #"mc16_13TeV.364172.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_BFilter.deriv.DAOD_"+derivation+".e5340_e5984_s3126_s3136_r10201_r10210_p3371",
        #"mc16_13TeV.364172.Sherpa_221_NNPDF30NNLO_Wenu_MAXHTPTV0_70_BFilter.deriv.DAOD_"+derivation+".e5340_s3126_r10201_r10210_p3371",
        ]
    if debug:
        print "rucio_all",rucio_all
    return rucio_all
# done function

def rucioFile(name):
    if debug:
        print "Start rucioFile()"
        print "name",name
    if doTestAfterRucio:
        return rucioFileTest(derivation)
    # if here, continue normally
    # there is a list of info, initially as for data17 there are two p-tags
    # the latest runs are only in the second p-tag
    # can also be used to have signal and background in the same mcdata list
    if option=="VHbb" and name.endswith("_e"):
        if derivation=="HIGG5D1" or derivation=="HIGG5D2":
            name="mcdata_e_0L1L"
        elif derivation=="HIGG2D4":
            name="mcdata_e_2L"
        else:
            print "derivation",derivation,"is not known for VHbb and period e. Chooose HIGG5D1, HIGG5D2, HIGG2D4, will ABORT!!!"
            assert(False)
    if verbose:
        print "name",name
    list_info = dict_name_list_info[name]
    if verbose:
        print "list_info",list_info
    rucio_info = {}
    for info in list_info:
        if verbose:
            print "info",info
        nameType = info[0]
        rtag     = info[1]
        ptag     = info[2]
        if verbose:
            print "nameType",nameType,"rtag",rtag,"ptag",ptag
        # e.g. rucio_input = "data15_13TeV.002762*.physics_Main.deriv.DAOD_HIGG5D1.r9264_p3083_p3372"
        rucio_input = nameType+"_13TeV.*.deriv.DAOD_"+derivation+".*"+rtag+"_"+ptag
        # With open(rucio_output, 'w') as rucio_out:
        #     rucio_out.write(subprocess.call(['rucio', 'ls', rucio_cmd]))
        # for future to try: to return only containers: rucio list-dids --filter 'type=CONTAINER' mc16_13TeV.364149.Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_CFilterBVeto.*.e5308_s3126_*
        if verbose:
            print('  Getting rucio info -> rucio ls {}'.format(rucio_input))
        try:
            proc = subprocess.Popen(['rucio', 'ls', '--short', rucio_input],
                                    stdout=subprocess.PIPE)
        except OSError:
            print('rucio is not configured. Cannot continue.')
            print('Did you forget to lsetup rucio?')
            sys.exit(1)
        rucio_info['{}_{}_{}'.format(nameType,rtag, ptag)] = proc.stdout.read()
    # join everything into a single long string with newline chars
    if debug:
        print "rucio_info",rucio_info
    all_rucio_info = '\n'.join(rucio_info.values())
    if debug:
        print "all_rucio_info"
        for line in all_rucio_info.split("\n"):
            print "line",line

    # remove the scope
    cleaned = []
    for line in all_rucio_info.split('\n'):
        try:
            cleaned.append(line.split(':')[1])
        except:
            # skip lines without scope (e.g. empty lines)
            continue

    # sort by DSID
    try:
        # sort by DSID, not good as it mixes data and MC, e.g.
        # data16_13TeV.00307358.physics_Main.deriv.DAOD_HIGG5D1.r9264_p3083_p3372
        # mc16_13TeV.307380.MadGraphPythia8EvtGen_A14NNPDF23LO_HVT_Agv1_VzWH_lvqq_m0300.deriv.DAOD_HIGG5D1.e5614_e5984_s3126_r9364_r9315_p3374
        # data16_13TeV.00307394.physics_Main.deriv.DAOD_HIGG5D1.r9264_p3083_p3372
        # sort = sorted(cleaned, key = lambda x: int(x.split('.')[1]))
        # instead, sort alphabeticaly, to have data first, then mc, and within data and mc automatically alphabetically implies also by DSID
        sort = sorted(cleaned)
    except:
        print('Warning: Sorting the list failed!')
    else:
        # use sorted list when no errors occured
        cleaned = sort

    if debug:
        "After sort in alphabetical order"
        print "cleaned"
        for onecleaned in cleaned:
            print onecleaned
    # done if
    return cleaned
# done function

def get_dict_DSID_list_dataset(rucio_all):
    dict_DSID_list_dataset={}
    for dataset in rucio_all:
        DSID=dataset.split(".")[1]
        if DSID in dict_DSID_list_dataset:
            dict_DSID_list_dataset[DSID].append(dataset)
        else:
            dict_DSID_list_dataset[DSID]=[dataset]
    # done loop over dataset resulted from rucio
    if debug:
        for DSID in sorted(dict_DSID_list_dataset):
            print DSID,dict_DSID_list_dataset[DSID]
    # done
    return dict_DSID_list_dataset
# done function

def get_dict_tagType_tags(AMITag):
    if debug:
        print "AMITag",AMITag
    # get AMITag, e.g.
    # AF: e6348_e5984_a875_r9364_r9315_p3371
    # FS: e6348_e5984_s3126_r9364_r9315_p3557
    list_AMITagComponent=AMITag.split("_")
    dict_tagType_tags={}
    for AMITagComponent in list_AMITagComponent:
        if debug:
            print "AMITagComponent",AMITagComponent
        tagType=AMITagComponent[0]
        if tagType in dict_tagType_tags.keys():
            dict_tagType_tags[tagType]+="_"+AMITagComponent
        else:
            dict_tagType_tags[tagType]=AMITagComponent
        # done if
    # done loop over components of AMITag
    if debug:
        print "dict_tagType_tags",dict_tagType_tags
    return dict_tagType_tags
# done function

def get_datasetProperties(dataset):
    if debug:
        print "dataset",dataset
    # 
    # get isMC
    if debug:
        print "dataset split",dataset.split(".")
    prefix=dataset.split(".")[0]
    if "mc" in prefix:
        isMC=1
    elif "data" in prefix:
        isMC=0
    else:
        print "neither mc nor data are present in dataset.split(\".\")[0]. Will ABORT!!!!"
        assert(False)
    # done if
    # 
    # get AMITag, e.g.
    # AF: e6348_e5984_a875_r9364_r9315_p3371
    # FS: e6348_e5984_s3126_r9364_r9315_p3557
    AMITag=dataset.split(".")[-1]
    if debug:
        print "AMITag",AMITag
    dict_tagType_tags=get_dict_tagType_tags(AMITag)
    if debug:
        print "AMITag",AMITag,"dict_tagType_tags", dict_tagType_tags
    if isMC:
        # MC sample
        # find out if this AMITag is FullSimulation (FS) or ATLAS Fast II (AF2)
        if "a" in dict_tagType_tags.keys():
            priority=0 # AF
        else:
            priority=1 # FS or data
        # done if
    else:
        # data run
        if "f" in dict_tagType_tags.keys():
            # "f961" -> -961
            # "f966" -> -966
            # visual inspection showed that when there are two f tags for a run
            # the neibourghing runs are of the lower f tag value
            # so we decide to kep the lower number
            # but later on we choose that with the highest priority
            # so we assign a negative sign, so the lowest f tag value has the highest priority
            priority=-int(dict_tagType_tags['f'][1:])
        else:
            # no f, so no special priority
            priority=0
    # done if
    if debug:
        print "priority",priority,type(priority)
    # get number of events
    # nrEvents=max(get_nrEvents_from_AMI(dataset),get_nrEvents_from_RUCIO(dataset))
    nrEvents=get_nrEvents_from_AMI(dataset)
    if debug:
        print "priority,nrEvents",(priority,nrEvents)
    return (priority,nrEvents,dict_tagType_tags)
# done function

def get_dict_DSID_list_datasetChosen(dict_DSID_list_dataset):
    dict_DSID_list_datasetChosen={}
    # list_considertags="e,s,a,r,f,m".split(",")
    # done not include for data the f and m, so that the various f tags for data18
    # to appear in the same considertag, to keep only one from it based on priority
    list_considertags="e,s,a,r".split(",")
    for DSID in sorted(dict_DSID_list_dataset):
        dict_DSID_list_datasetChosen[DSID]=[]
        list_dataset=dict_DSID_list_dataset[DSID]
        if debug or verbose:
            print "DSID",DSID
        # loop over datasets in list and choose only one
        # compute the quantities only once, to save CPU
        # especially the number of events needs to run rucio and pyami
        # which can be slower
        # so we cache the result
        dict_dataset_datasetProperties={}
        dict_considertag_list_dataset={}
        for dataset in list_dataset:
            properties=get_datasetProperties(dataset)
            if debug:
                print "dataset",dataset
                print "properties",properties
            dict_dataset_datasetProperties[dataset]=properties
            considertag=""
            for i,tag in enumerate(list_considertags):
                if i>0:
                    considertag+="_"
                if tag in properties[2].keys():
                    considertag+=properties[2][tag]
                else:
                    considertag+="None" # should be the case for data
                # done if
            # done for loop over tags
            if considertag in dict_considertag_list_dataset.keys():
                dict_considertag_list_dataset[considertag].append(dataset)
            else:
                dict_considertag_list_dataset[considertag]=[dataset]
            # done if
        # done for loop
        # keep only one dataset for each considertag
        if debug:
            print ""
            print "loop over considertag for this DSID:"
        for considertag in sorted(dict_considertag_list_dataset.keys()):
            if debug or verbose:
                print ""
                print "considertag",considertag
            list_dataset=dict_considertag_list_dataset[considertag]
            if debug:
                print "list_dataset",list_dataset
            datasetChosen=list_dataset[0]
            if debug:
                print "datasetChosen",datasetChosen
            datasetChosenProperties=dict_dataset_datasetProperties[datasetChosen]
            if debug:
                print "datasetChosenProperties",datasetChosenProperties
            # loop over all the datasets
            # use algorithm similar to sorting numbers
            # if new dataset has a higher priority than the previous dataset
            # assign to datasetChosen the current dataset
            # higher priority is if FS and the chosen is AF
            # or if same FS or AF, by the larger ptag
            for dataset in list_dataset:
                if debug:
                    print "dataset",dataset
                datasetProperties=dict_dataset_datasetProperties[dataset]
                if debug:
                    print "datasetProperties",datasetProperties
                if debug or verbose:
                    print "dataset",dataset,"nrEvents",datasetProperties[1]
                if debug:
                    print "datasetProperties[0]",datasetProperties[0],"datasetChosenProperties[0]",datasetChosenProperties[0]
                # do the ordering to get the latest ptag
                if datasetProperties[0]>datasetChosenProperties[0]:
                    # if the priority (simulation, or negative f-tag value) is strictly larger
                    # i.e. the previous is AF and the new one is FS
                    # make the new one as default
                    datasetChosen=dataset
                    datasetChosenProperties=dict_dataset_datasetProperties[datasetChosen]
                else:
                    # then the simulation is exactly the same
                    # i.e. either still AF, or still FS
                    if datasetProperties[1]>datasetChosenProperties[1]:
                        # if the number of events is larger than in previous
                        # make the new one as default
                        # note: not necessarily the largest ptag value (datasetProperties[1])
                        datasetChosen=dataset
                        datasetChosenProperties=dict_dataset_datasetProperties[datasetChosen]
                    # done if
                # done if
            # done for loop over dataset
            # we have found the chosen dataset
            # add it to the list only if number events non zero
            if debug:
                print "After compariston:"
                print "datasetChosen",datasetChosen
            datasetChosenProperties=dict_dataset_datasetProperties[datasetChosen]
            if debug:
                print "datasetChosenProperties",datasetChosenProperties
            if datasetChosenProperties[1]>0:
                if debug:
                    print "Number of events larger than zero, so declar as chosen"
                dict_DSID_list_datasetChosen[DSID].append(datasetChosen)
            # done if
            if debug:
                print "for this DSID",DSID,"this list of datasets has been chosen:"
                print dict_DSID_list_datasetChosen[DSID]
        # done loop over considertag for a give DSID
        # dict_DSID_list_datasetChosen[DSID] is done
        # we do one more check. If one of them is FS, remove all AF
        atLeastOneFS=False
        for datasetChosen in dict_DSID_list_datasetChosen[DSID]:
            datasetChosenProperties=dict_dataset_datasetProperties[datasetChosen]
            if debug:
                print "datasetChosen",datasetChosen,"datasetChosenProperties",datasetChosenProperties
            if datasetChosenProperties[0]==1:
                atLeastOneFS=True
        if atLeastOneFS:
            list_datasetChosen=[]
            # loop over the chosenDataset and keep only those that are FS
            for datasetChosen in dict_DSID_list_datasetChosen[DSID]:
                datasetChosenProperties=dict_dataset_datasetProperties[datasetChosen]
                if datasetChosenProperties[0]==1:
                    list_datasetChosen.append(datasetChosen)
            # now replace the initial list with the new list
            dict_DSID_list_datasetChosen[DSID]=list_datasetChosen
        # done if
    # done for loop over DSID
    if debug:
        print "Showing list of chosenDataset for each DSID:"
        for DSID in sorted(dict_DSID_list_datasetChosen):
            print "DSID",DSID
            list_datasetChosen=dict_DSID_list_datasetChosen[DSID]
            for datasetChosen in list_datasetChosen:
                print "datasetChosen",datasetChosen
    # done
    return dict_DSID_list_datasetChosen
# done function

def get_GRLFileName(sampleType):
    if debug:
        print "Start get_GRLFileName"
        print "sampleType",sampleType
    if "data" in sampleType:
        GRLFileName = pathGRL+"/"+dict_sampleType_GRL[sampleType]
    else:
        GRLFileName = None
    if debug:
        print "GRLFileName",GRLFileName
    return GRLFileName
# done function

def get_listGRL(GRLFileName):
    if debug:
        print "Start get_listGRL()"
        print "GRLFileName",GRLFileName
    if GRLFileName is None: # for MC
        return None
    # Typical in our GRL .xml file
    # <Metadata Name = "RQTSVNVersion">DQDefects-00-02-02</Metadata>
    # <Metadata Name = "RunList">276262,276329,276336,276416,276511,276689,276778,276790,276952,276954,278880,278912,278968,279169,279259,279279,279284,279345,279515,279598,279685,279813,279867,279928,279932,279984,280231,280273,280319,280368,280423,280464,280500,280520,280614,280673,280753,280853,280862,280950,280977,281070,281074,281075,281317,281385,281411,282625,282631,282712,282784,282992,283074,283155,283270,283429,283608,283780,284006,284154,284213,284285,284420,284427,284484</Metadata>
    root = ET.parse(GRLFileName).getroot()
    root = root[0]
    for a in root.findall('Metadata'):
        if "RunList" in a.attrib.values():
            stringGRL = a.text
    if debug:
        print "stringGRL",stringGRL
    listGRL = stringGRL.split(",")
    if debug:
        print "listGRL",listGRL
    return listGRL
# done function

def get_sampleType(dataset):
    if debug:
        print "Start get_sampleType()"
        print "dataset",dataset
    sampleType=dataset.split("_")[0]
    if debug:
        print "sampleType",sampleType
    return sampleType
# done function

def get_runNumber(dataset):
    if debug:
        print "Start get_runNumber()"
        print "dataset",dataset
    list_element = dataset.split(".")
    runNumber = list_element[1]
    # ignore the first two elements, which are "00"
    runNumber = runNumber[2:]
    if debug:
        print "runNumber",runNumber
    return runNumber
# done function

def get_isDatasetInGRL(dataset,dict_sampleType_listGRL):
    if debug:
        print "get_isDatasetInGRL()"
        print "dataset",dataset
    sampleType=get_sampleType(dataset)
    listGRL=dict_sampleType_listGRL[sampleType]
    if debug:
        print "listGRL",listGRL
    runNumber = get_runNumber(dataset)
    if "data" in sampleType:
        result = runNumber in listGRL
    else:
        # for MC it always returns true
        result = True
    if debug:
        print "isDatasetInGRL",result,"dataset",dataset
    return result
# done function

def get_nrEvents_from_AMI_once(dataset,debug):
    if debug:
        print "Start get_nrEvents_from_AMI()"
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
            print "AMI:","var",var,"value",dict_var_value[var]
    # dataset is good if it has at least some events
    # we may add more requirements in the future
    good=True
    good=good and ("amiStatus" in dict_var_value.keys() and dict_var_value["amiStatus"]=="VALID")
    good=good and ("taskStatus_0" in dict_var_value.keys() and (dict_var_value["taskStatus_0"]=="DONE" or dict_var_value["taskStatus_0"]=="FINISHED"))
    if checkInAMIAllEventsAreAvailable==True:
        good=good and ("prodsysStatus" in dict_var_value.keys() and dict_var_value["prodsysStatus"]=="ALLEVENTSAVAILABLE")
    good=good and ("totalEvents" in dict_var_value.keys())
    if good==True:
        result=int(dict_var_value["totalEvents"])
    else:
        result=0
    # done if
    if debug:
        print "nrEvents",result,"dataset",dataset
    return result
# done function

def get_nrEvents_from_AMI(dataset):
    result=0
    counter=0
    number_retries=1
    # sometimes AMI retruns zero events even if in reality it is correct when retried
    # so if zero, let's retry a few times just to catch the case of AMI errors
    while counter<number_retries+1:
        if counter>=1:
            my_debug=True
            os.system("sleep 1")
        else:
            my_debug=False
        result=get_nrEvents_from_AMI_once(dataset,my_debug)
        if result==0 or my_debug:
            print "ADRIAN counter AMI retrial",counter,"number events in AMI",result
        if result>0:
            break
        counter+=1
    # done while
    return result
# done function

def get_nrEvents_from_RUCIO(dataset):
    if debug:
        print "Start get_nrEvents_from_RUCIO()"
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

def get_isDatasetGood(dataset):
    if debug:
        print "Start get_isDatasetGood()"
        print "dataset",dataset
    # if we want that either rucio or AMI to be non zero, to select the largest samples that really exist
    # nrEvents=max(get_nrEvents_from_AMI(dataset),get_nrEvents_from_RUCIO(dataset))
    # keep only AMI if we want to be sure that they are finished, to not run on derivations that exist but are not completed
    nrEvents=get_nrEvents_from_AMI(dataset)
    isDatasetGood=(nrEvents>0)
    if debug:
        print "nrEvents",nrEvents,"isDatasetGood",isDatasetGood
    return isDatasetGood
# done function

def get_isDatasetUseful(dataset):
    if debug:
        print "Start get_isDatasetUseful()"
        print "dataset",dataset
    result = True
    for notUseful in list_notUseful:
        found=match_all_substrings_in_string(notUseful,dataset,debug=False)
        if found==True:
            result = False
    # done for loop
    if debug:
        print "isDatasetUseful",result,"dataset",dataset
    return result
# done function

def get_keepForThisDSID(dataset,dict_DSID_list_datasetChosen):
    if debug:
        print "Start get_keepForThisDSID(dataset,dict_DSID_list_datasetChosen)"
        print "dataset",dataset
    DSID=dataset.split(".")[1]
    list_datasetChosen=dict_DSID_list_datasetChosen[DSID]
    if debug:
        print "list_datasetChosen",list_datasetChosen
    if dataset in list_datasetChosen:
        result=True
    else:
        result=False
    if debug:
        print "result",result
    return result
# done function

def commentFileForEmptyContainerOrNotInGRL(name, file_contents):
    if debug:
        print "Start commentFileForEmptyContainerOrNotInGRL()"
        print "name",name
        print "file_contents",file_contents
    if doNotCommentOut==True:
        if verbose:
            print "doNotCommentOut=True, so returning directly file_contents"
        return file_contents
    # done if
    dict_sampleType_listGRL={}
    for sampleType in "data15,data16,data17,data18,mc16".split(","):
        if debug:
            print "sampleType",sampleType
        if sampleType=="mc16":
            listGRL=[]
            if debug:
                print "sampleType=mc16 so listGRL is empty"
        else:
            if debug:
                print "sampleType is not mc16, so expected to be one of the data years"
            GRLFileName = get_GRLFileName(sampleType)
            if debug:
                print "GRLFileName",GRLFileName
            listGRL = get_listGRL(GRLFileName)
            if debug:
                print "listGRL",listGRL
        # done if
        if debug:
            print "done if mc16 or else (data) the result is listGRL",listGRL
        dict_sampleType_listGRL[sampleType]=listGRL
    # done for loop over sampleType
    if debug:
        print "dict_sampleType_listGRL:"
        for sampleType in dict_sampleType_listGRL.keys():
            print "sampleType",sampleType,"listGRL",dict_sampleType_listGRL[sampleType]
    # get dictionary of DSID vs list of datasets
    dict_DSID_list_dataset=get_dict_DSID_list_dataset(file_contents)
    if debug:
        print "dict_DSID_list_dataset",dict_DSID_list_dataset
    dict_DSID_list_datasetChosen=get_dict_DSID_list_datasetChosen(dict_DSID_list_dataset)
    if debug:
        print "dict_DSID_list_datasetChosen",dict_DSID_list_datasetChosen
    # loop over the dataset in the listFile
    commented_contents = []
    for dataset in file_contents:
        dataset = dataset.rstrip()
        # first remove all the comments already from the file and start with a clean uncommented list
        dataset = dataset.replace("#","")
        sampleType=dataset.split("_")[0]
        if debug:
            print "dataset",dataset,"sampleType",sampleType
        # evaluate if we want to keep this dataset
        # we need an and between several booleans
        # to make it faster, we check one only if the previous was passed
        keepDataset=False
        isDatasetInGRL=get_isDatasetInGRL(dataset,dict_sampleType_listGRL)
        if debug:
            print "isDatasetInGRL",isDatasetInGRL            
        if isDatasetInGRL:
            # useful dataset if not in list_notUseful defined at the top, such as some mono-Hbb signals using the same derivations
            isDatasetUseful=get_isDatasetUseful(dataset)        
            if debug:
                print "isDatasetUseful",isDatasetUseful                
            if isDatasetUseful:
                # check if we keep this DSID
                keepForThisDSID=get_keepForThisDSID(dataset,dict_DSID_list_datasetChosen)
                if debug:
                    print "keepForThisDSID",keepForThisDSID
                if keepForThisDSID:
                    keepDataset=True
                else:
                    reasonFail="dataset not kept"
                # done if
            else:
                reasonFail="dataset not useful"
            # done if
        else:
            reasonFail="dataset not in GRL"
        # done if
        # keepDataset = True # temporary pass all for quicker test
        if debug:
            print "keepDataset",keepDataset
        if keepDataset:
            if verbose:
                print('  GRL & AMI success:          {}'.format(dataset))
            commented_contents.append(dataset)
        else:
            if verbose:
                print('  GRL & AMI fail:             {}'.format(dataset))
                print('  failed for reason: {}.'.format(reasonFail))
            commented_contents.append('# ' + dataset)
    return commented_contents
# done function

def applyVetosFromSampleInfo(file_contents):
    if debug:
        print "Start applyVetosFromSampleInfo()"
        print "file_contents",file_contents
    workdir = os.getenv('WorkDir_DIR')
    if not workdir or not os.path.isdir(workdir):
        print('Warning: WorkDir_DIR is not set. Cannot apply vetoes from sample_info.txt')
        return file_contents
    datadir = os.path.join(workdir, '..', '..', 'source', 'CxAODOperations_VHbb', 'data', 'DxAOD', 'info')
    if not os.path.isdir(datadir):
        print('Warning: Cannot locate $WorkDir_DIR/../../source/CxAODOperations_VHbb/data/DxAOD/info')
        return file_contents
    infofile = os.path.join(datadir, 'sample_info.txt')
    if not os.path.isfile(infofile):
        print('Warning: {} is not a valid file'.format(infofile))
    vetos = []
    with open(infofile) as f:
        for line in f.readlines():
            try:
                dsid = int(line.split()[0])
            except:
                # ignore empty or commented lines
                continue
            if 'veto' in line:
                vetos.append(dsid)

    # compile list with vetos
    vetoed = []
    for line in file_contents:
        if line.startswith('#'):
            # it's already commented, but we still keep it!
            vetoed.append(line)
            continue
        dsid = int(line.split('.')[1])
        if dsid in vetos:
            if verbose:
                print('  Vetoed by sample_info.txt:  {}'.format(line))
            vetoed.append('# ' + line)
        else:
            vetoed.append(line)
    return vetoed
# done function

def writeToFile(name, listFileName, contents):
    if debug:
        print "Start writeToFile()"
    """
    Write all contents to `listFileName`.
    Take into account that if `name` is equal to "signal" we want to split
    into the different signal categories, like SM, DBL, etc.
    Make sure the resulting file has a trailing newline.
    """
    if not 'signal' in name:
        if not contents:
            print('  --> Warning: Empty contents for file {}'.format(listFileName))
            return
        if verbose:
            print('  --> Writing into {}'.format(listFileName))
        if debug:
            print "contents",contents
        with open(listFileName, 'w') as out:
            out.write('\n'.join(contents))
            out.write('\n')
    else:
        # 1) write all that is signal
        # -> veto everything that is in signal exceptions
        if verbose:
            print('  --> Splitting output into several lists: {}'
                  .format(['signal'] + signal_exceptions.keys()))
        vetoed = []
        for c in contents:
            veto = False
            for exc_list in signal_exceptions.values():
                for s in exc_list:
                    found=match_all_substrings_in_string(s,c,debug)
                    if found==True:
                        veto = True
            if not veto:
                vetoed.append(c)
        if verbose:
            print('  --> Writing into {}'.format(listFileName))
        if debug:
            print "vetoed",vetoed
        with open(listFileName, 'w') as out:
            out.write('\n'.join(vetoed))
            out.write('\n')
        if len(vetoed)==0:
            os.system('rm -f '+listFileName)

        # 2) write all others
        # loop over all signal exceptions, do substring matching with anything
        # from substring list of current signal exception
        for exc_sample, exc_list in signal_exceptions.iteritems():
            modifiedListFileName = listFileName.replace('signal', exc_sample)
            matched = []
            for c in contents:
                match = False
                for exc in exc_list:
                    found=match_all_substrings_in_string(exc,c,debug)
                    if found==True:
                        match = True
                if match:
                    matched.append(c)
            if verbose:
                print('  --> Writing into {}'.format(modifiedListFileName))
            if debug:
                print "matched",matched
            with open(modifiedListFileName, 'w') as out:
                out.write('\n'.join(matched))
                out.write('\n')
            if len(matched)==0:
                os.system('rm -f '+modifiedListFileName)
# done function

#################################################################
################### Run #########################################
#################################################################

if __name__ == '__main__':
    for derivation in list_derivation:
        if debug:
            print "derivation",derivation
        doIt(list_name, derivation)

#################################################################
################### Finished ####################################
#################################################################

print ""
print ""
print "Finished createSampleList.py with settings:"
if True:
    print "option",option
    print "list_channel",list_channel
    print "list_mcType",list_mcType 
print "Done All."
exit()
