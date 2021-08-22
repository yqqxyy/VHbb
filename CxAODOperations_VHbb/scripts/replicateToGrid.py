#!/usr/bin/env python
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group
#
# Replicate output dataset list to group production grid space (for which you need production role)
# If you use the "all" option there will be no checks of the yields files done at all
# Easiest is to set up your grid proxy by hand before e.g. for Higgs: 
#  voms-proxy-init -voms atlas:/atlas/phys-higgs/Role=production
# Paul Thompson (thompson@mail.cern.ch)

import os, math, sys

total = len(sys.argv)
# number of arguments plus 1
if total!=4:
  print "You need some arguments, will ABORT!"
  print "Usage:",sys.argv[0],"priviledge           option         out_sample_list                                                        "
  print "Usage:",sys.argv[0],"physgroup=phys-higgs all            out_sample_list_sample_grid.13TeV_25ns.mcdata_d.HIGG5D1.txt"
  print "Usage:",sys.argv[0],"physgroup=phys-higgs only100Percent out_sample_list_sample_grid.13TeV_25ns.mcdata_d.HIGG5D1.txt"
  assert(False)
# done if
 
# inputs
priviledge  = sys.argv[1]
option      = sys.argv[2]
in_file     = sys.argv[3]
debug       = True
nrReplicas  = "1"

##############################
# rucio documentation :
# https://twiki.cern.ch/twiki/bin/view/AtlasComputing/RucioClientsHowTo
# example of commands to run by hand
# find out where the dataset exists
# rucio list-dataset-replicas group.phys-higgs.mc16_13TeV.364173.CAOD_HIGG5D1.e5340_s3126_r9364_r9315_p3371.31-10_CxAOD.root
# export RUCIO_ACCOUNT=phys-higgs
# replicate one copy to any RSE that belongs to the physgroup=phys-higgs
# rucio add-rule group.phys-higgs.mc16_13TeV.364173.CAOD_HIGG5D1.e5340_s3126_r9364_r9315_p3371.31-10_CxAOD.root 1 physgroup=phys-higgs
# short cut for doing by hand
# A=user.trashid.mc16_13TeV.361031.CAOD_HIGG5D2.e3569_s3126_r9364_r9315_p3371.31-10
# rucio add-rule ${A}_CxAOD.root 1 physgroup=phys-higgs && rucio add-rule ${A}_hist 1 physgroup=phys-higgs
##############################

####################################################
### Configurations #################################
####################################################

# very improtant not to have / in the end of dataset name
list_suffix = [
#"_log", # log files
"_CxAOD.root", # CxAOD
"_hist", # histograms with cut flow
]

####################################################
### Functions ######################################
####################################################

def runCommand(info,command,verbose=True,run=True):
  if verbose:
    print "Start ",info,"command:",command
  if run:
    os.system(command)
  if verbose:
    print "End  ",info,"command:",command
# done function

def countYieldsAndCheckYields():
  runCommand("countYieldsForAMI","./count_Nentry_SumOfWeight.py 1 0 0")
  runCommand("checkYields","./checkYields.py")
  runCommand("listThoseNotFullyDownloaded","grep -v 1.000 dslist_NevtDxAOD_yield.txt")
  # runCommand("countYieldsForReader","./count_Nentry_SumOfWeight.py 0 0 1")
# done function

def get_list_DSIDAMITag_that_is_Complete():
  inputFileName="dslist_NevtDxAOD_yield.txt"
  if not os.path.isfile(inputFileName):
    print "file",inputFileName,"does not exist, so first count yields and then check yields"
    countYieldsAndCheckYields()
  # now the file exists, so we can open it
  inputFile=open(inputFileName,"r")
  list_DSIDAMITag=[]
  for line in inputFile:
    # e.g. 00335016   1.000    f868_m1870_p3372                                             physics_Main 
    # e.g. 364168     1.000    e5340_e5984_s3126_s3136_r10201_r10210_p3371                  Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV500_1000               
    # e.g. 364168     1.000    e5340_s3126_r10201_r10210_p3371                              Sherpa_221_NNPDF30NNLO_Wmunu_MAXHTPTV500_1000  
    list_line=line.split()
    DSID    =list_line[0]
    fraction=list_line[1]
    AMITag  =list_line[2]
    name    =list_line[3]
    if option=="only100Percent":
      if fraction!="1.000":
        continue
    elif option=="all":
      None
    else:
      print "option",option,"not known. It should be all or only100Percent. Will ABORT!"
      assert(False)
    # done if
    list_DSIDAMITag.append(DSID+"."+AMITag)
  # done loop over lines
  if debug:
    print "list_DSIDAMITag"
    for DSIDAMITag in list_DSIDAMITag:
      print "DSIDAMITag",DSIDAMITag
  return list_DSIDAMITag
# done function

def set_RUCIO_ACCOUNT(priviledge):
  print "Initially Setting RUCIO_ACCOUNT="+os.environ["RUCIO_ACCOUNT"]
  if "physgroup" in priviledge:
    group=priviledge.split("=")[1]
    print "You are running as a physgroup="+group
    os.environ["RUCIO_ACCOUNT"] = group
    print "so setting RUCIO_ACCOUNT="+os.environ["RUCIO_ACCOUNT"]
# done function

def replicateToGrid():
  # only check for yields if all option is not used
  list_DSIDAMITag = get_list_DSIDAMITag_that_is_Complete() if option !="all" else []
  if option !="all" and len(list_DSIDAMITag)==0:
    print "There is some error, or no sample is finished at 1.000, so we do not replicate anything"
    return
  # at least one DSID to replicate
  read_file = open('./'+in_file, 'r')
  for line in read_file :
    sample = line.rstrip()
    # protect from commented lines or empty 
    if sample.find("#")>=0:
      continue
    if sample=="":
      continue
    # check if not completed only if the file exist
    DSID=sample.split(".")[3]
    AMITag=sample.split(".")[5]
    DSIDAMITag=DSID+"."+AMITag
    if option!="all" and DSIDAMITag not in list_DSIDAMITag:
      continue
    # this sample is one that is already completed
    # we'll create a replication rule for it
    # if it has been replicated already, nothing will be done
    if debug:
      print "Doing now DSIDAMITag",DSIDAMITag
    for suffix in list_suffix:
      #command+=" "+sample+suffix
      command="sleep 1.0 && rucio list-dataset-replicas "+sample+suffix
      print "command="+command
      os.system(command)
      command="rucio add-rule "+sample+suffix+" "+nrReplicas+" "+priviledge
      print "command="+command
      os.system(command)
    # done loop over suffix
  # done loop over samples in output list file
  #print "command:"
  #print command
  #os.system(command)
  
  print ""
  print ""
  print "If you get the error message: There is not enough quota left to fulfil the operation. Details: T"
  print "Check that RUCIO_ACCOUNT is not your CERN username as default, but that of the physics group, like phys-higgs if you asked that priviledge."
  print "All replicateToGrid finished fine"
# done function

####################################################
#### Run ###########################################
####################################################

set_RUCIO_ACCOUNT(priviledge)
replicateToGrid()
  
####################################################
#### Finished ######################################
####################################################

print ""
print ""
print "All finished fine."
