#!/usr/bin/env python
# Expanded by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group
import os, math, sys

total = len(sys.argv)
# number of arguments plus 1
if total!=2:
  print "You need some arguments, will ABORT!"
  print "Usage:",sys.argv[0],"out_sample_list                                                         "
  print "Usage:",sys.argv[0],"out_sample_list_sample_grid.mc15c_13TeV_25ns_signal_nominal_HIGG2D4.txt "
  assert(False)
# done if
 
# inputs
in_file = sys.argv[1]
debug=True

##############################
# rucio documentation :
# https://twiki.cern.ch/twiki/bin/view/AtlasComputing/RucioClientsHowTo
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
  runCommand("countYieldsForReader","./count_Nentry_SumOfWeight.py 0 0 1")
# done function

def get_list_DSIDAMITag_that_is_NotComplete():
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
    if fraction=="1.000":
      continue
    list_DSIDAMITag.append(DSID+"."+AMITag)
  # done loop over lines
  if debug:
    print "list_DSIDAMITag"
    for DSIDAMITag in list_DSIDAMITag:
      print "DSIDAMITag",DSIDAMITag
  return list_DSIDAMITag
# done function

def download():
  list_DSIDAMITag=get_list_DSIDAMITag_that_is_NotComplete()
  if len(list_DSIDAMITag)==0:
    print "There is some error, or all files are fine, so we do not download anything"
    return
  # done if
  #return
  #command="rucio download"
  # at least one DSID to download
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
    if DSIDAMITag not in list_DSIDAMITag:
      continue
    # this sample is one that is not completed yet
    # we download it
    for suffix in list_suffix:
      #command+=" "+sample+suffix
      command="sleep 1.0 && rucio download "+sample+suffix
      print "command="+command
      os.system(command)
    # done loop over suffix
  # done loop over samples in output list file
  #print "command:"
  #print command
  #os.system(command)
  print ""
  print ""
  print "All download finished fine"
# done function


####################################################
#### Run ###########################################
####################################################

# countYieldsAndCheckYields()
download()
countYieldsAndCheckYields()

####################################################
#### Finished ######################################
####################################################

print ""
print ""
print "All finished fine."
