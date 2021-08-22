#!/usr/bin/env python

import os, math, sys

total = len(sys.argv)
# number of arguments plus 1
if total!=6:
  print "You need some arguments, will ABORT!"
  print "Usage: ",sys.argv[0]," user             vtag  file_in                                                                          folder_out       debug"
  print "Usage: ",sys.argv[0]," user.abuzatu     31-01 ./FrameworkSub/In/CxAOD30/list_sample_grid.test_c_13TeV_25ns_nominal.HIGG5D1.txt ForCxAODDownload 0"
  print "Usage: ",sys.argv[0]," group.phys-higgs 31-01 ./FrameworkSub/In/CxAOD30/list_sample_grid.test_c_13TeV_25ns_nominal.HIGG5D1.txt ForCxAODDownload 0"
  assert(False)
# done if

# inputs
user              = sys.argv[1]
vtag              = sys.argv[2]
file_in_with_path = sys.argv[3]
folder_out        = sys.argv[4]
debug             = bool(int(sys.argv[5]))

if debug:
  print "user",user
  print "vtag",vtag
  print "file_in_with_path",file_in_with_path
  print "folder_out",folder_out

# if / in the end, remove it
if file_in_with_path[-1]=='/':
  file_in_with_path=file_in_with_path[:-1]

sample_file  = open(file_in_with_path, 'r')

# output file, to take back jobs
file_in=file_in_with_path.split("/")[-1]
out_file = open(folder_out+'/out_sample_'+file_in, 'w')

# loop over samples in file
for sampleIn in sample_file :
  # protect from commented lines
  if sampleIn.find("#")>=0 : continue
  # remove end of lines
  sampleIn=sampleIn.rstrip()
  # protect from blank lines
  if sampleIn == "\n" : continue
  # 
  if debug:
    print "sampleIn",sampleIn
  # split in elements
  list_sampleElement=sampleIn.split(".")
  if len(list_sampleElement)!=6:
    print "list_sampleElement",list_sampleElement,"does not have 6 elements. Will ABORT!!!"
    assert(False)
  sampleType    =list_sampleElement[0] # e.g. data17_13TeV or mc16_13TeV
  sampleDSID    =list_sampleElement[1] # e.g. 00327490 or 345058
  sampleName    =list_sampleElement[2] # Main.deriv or Sherpa_221_NNPDF30NNLO_Znunu_MAXHTPTV140_280_BFilter
  sampleDeriv   =list_sampleElement[3] # deriv
  sampleDAOD    =list_sampleElement[4] # DAOD_HIGG5D1
  sampleAMITags =list_sampleElement[5] # f838_m1824_p3372 or e6004_e5984_s3126_r9781_r9778_p3374
  # calculate sampleOut
  sampleCAOD    = sampleDAOD.replace("DAOD","CAOD")
  sampleOut = user+"."+sampleType+"."+sampleDSID+"."+sampleCAOD+"."+sampleAMITags+"."+vtag
  if debug:
    print "sampleOut",sampleOut
  # add sampleOut to file
  out_file.write(sampleOut+"\n")

  # check length
  full_name = sampleOut.strip()
  #full_name += "_CxAOD.root"
  max_str = 115
  if len(full_name) >= max_str:
    print "WARNING : "+full_name+" output name is "+str(len(full_name))+" chars, it will crash the job (should be <"+str(max_str)+"). Re-visit the name structure."
# done loop over sampleIn

sample_file.close()
out_file.close()

print "Done form_HSG5_out.py"
