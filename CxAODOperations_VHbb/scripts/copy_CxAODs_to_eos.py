#!/usr/bin/env python
# Expanded by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis

import os, math, sys

total = len(sys.argv)
# number of arguments plus 1
 
if total!=8:
  print "You need some arguments, will ABORT!"
  print "Usage:",sys.argv[0],"doTestOnly doVerbose doSkipExisting doMakeSybmolicLink derivationTag CxAODTag  destinationFolder"
  print "Usage: Note the importance of / at the end of destination folder!"
  print "Usage: tag 28" 
  print "Eos for tag 28 described at Google Docs https://docs.google.com/spreadsheets/d/1IY5sFvnnf8_K0TUXQ7w3endBzPhTEoU6j4Lt-I1noHQ/edit#gid=0"
  print "Eos for tag 28 described also at Twiki https://twiki.cern.ch/twiki/bin/view/AtlasProtected/CxAODFramework#NEW_r28_01_in_Git"
  print "Usage:",sys.argv[0],"If not on eos, but on the same disk where you downloaded, you may want to create just sybmolic links use 0 1 1 1 " 
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1 00-28-01 /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_28/" # 0lep
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2 00-28-01 /eos/atlas/unpledged/group-tokyo/users/yenari/20170214/" # 1lep
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4 00-28-01 /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r28-01/" # 2lep
  print "Usage: You can copy also locally, not only on the eos. For locally, you have the choice to copy, or just make symbolic link. Useful if copying on the same machine."
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4 00-28-01 /data05/abuzatu/CxAOD/All/" # 2lep
  print "Usage: tag 29"
  print "Usage: description in Twiki: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/Release21Migration#Tag_r29_00_on_23_Nov"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1 00-29-00 /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_29/" # 0lep
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2 00-29-00 /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/171123_r29-00/" # 1lep
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4 00-29-00 /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/171123_r29-00/" # 2lep 
  print "Usage: tag 30"
  print "Usage: description in Twiki: https://twiki.cern.ch/twiki/bin/view/AtlasProtected/Release21Migration#NEW_Tag_r30_02_on_16_Nov"
  print "Usage: a (mc16a, data15, data16)"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1 00-30-02_a /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_30/" # 0lep a 
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2 00-30-02_a /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/180116_r30-02/" # 1lep a 
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4 00-30-02_a /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/180116_r30-02/" # 2lep a
  print "Usage: c (mc16c, data17)"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1 00-30-02_c /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_30/" # 0lep c
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2 00-30-02_c /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/180116_r30-02/" # 1lep c
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4 00-30-02_c /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/180116_r30-02/" # 2lep c
  print "Usage: tag 31-01 (v31 without systematics) "
  print"Usage: Twiki https://twiki.cern.ch/twiki/bin/view/AtlasProtected/Release21Migration#Tag_r31_01_on_26_March"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1    31-01_a /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1    31-01_c /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1    31-01_d /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2    31-01_a /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/CxAOD_31"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2    31-01_c /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/CxAOD_31"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2    31-01_d /eos/atlas/unpledged/group-tokyo/users/tatsuya/CxAOD/CxAOD_31"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4    31-01_a /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4    31-01_c /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4    31-01_d /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/"
  print "Usage: Adrian on ipas06 all tags in one place"
  print "Usage:",sys.argv[0],"0 1 1 1 HIGG5D1    31-01_a /data06/abuzatu/data/CxAOD/ToUseInReader"
  print "Usage:",sys.argv[0],"0 1 1 1 HIGG5D1    31-01_d /data06/abuzatu/data/CxAOD/ToUseInReader"
  print "Usage: tag 31-10 (v31 with systematics) "
  print "Usage: Twiki https://twiki.cern.ch/twiki/bin/view/AtlasProtected/Release21Migration#NEW_Tag_r31_11_on_30_April_CxAOD"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1    31-10_a /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r31-10/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1    31-10_d /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r31-10/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2    31-10_a /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r31-10/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2    31-10_d /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r31-10/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4    31-10_a /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r31-10/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4    31-10_d /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r31-10/"
  print "Usage: tag 31-15 (v31 without systematics) "
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4    31-15_a /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r31-15/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4    31-15_d /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r31-15/"
  print "Usage: Intermediate stages of validation for example for PERIOD=d and VTAG=181009"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D1    ${VTAG}_${PERIOD} /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r${VTAG}/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG5D2    ${VTAG}_${PERIOD} /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r${VTAG}/"
  print "Usage:",sys.argv[0],"0 1 1 0 HIGG2D4    ${VTAG}_${PERIOD} /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r${VTAG}/"
  assert(False)
# done if

# inputs
debug=True
doCreateAllFoldersEvenIfYouDoNotHaveSamplesDownloaded=False
doTestOnly=bool(int(sys.argv[1]))
doVerbose=bool(int(sys.argv[2]))
doSkipExisting=bool(int(sys.argv[3]))
doMakeSymbolicLink=bool(int(sys.argv[4]))
derivationTag=sys.argv[5]
productionTag=sys.argv[6]
destinationFolder=sys.argv[7]

#############################
#
# Script for copying grid-job output-files (CxAODs) to EOS.
#
# Sub-directories are created on EOS to group the files into samples.
#
# VERY IMPORTANT :
# Execute in the directory where you have downloaded your jobs from the grid with rucio.
#
# example full command
# xrdcp group.phys-higgs.data17_13TeV.00338220.CAOD_HIGG5D1.f877_m1885_p3372.31-01-7_CxAOD.root/group.phys-higgs.13626784._000030.CxAOD.root root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/HIGG5D1_13TeV/CxAOD_31-01_d/data17/group.phys-higgs.data17_13TeV.00338220.CAOD_HIGG5D1.f877_m1885_p3372.31-01-7_CxAOD.root/group.phys-higgs.13626784._000030.CxAOD.root
# short way when you have to copy only a few files by hand when they are missing or problematic on from eos
# E=root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/HIGG5D1_13TeV/CxAOD_31-01_d
# F=group.phys-higgs.data17_13TeV.00338220.CAOD_HIGG5D1.f877_m1885_p3372.31-01-7_CxAOD.root/group.phys-higgs.13626784._000030.CxAOD.root
# xrdcp $F $E/data17/$F
# for the next file define F again and then run again xrdcp $F $E/data17/$F
# to make a folder
# xrd eosatlas.cern.ch mkdir /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/HIGG5D2_13TeV/CxAOD_31-01_a/sigDBL/$F
# to make a folder and copy all the files inside
# F=group.phys-exotics.mc16_13TeV.307587.CAOD_HIGG5D2.e5707_e5984_s3126_r9364_r9315_p3374.31-01_CxAOD.root/
# xrd eosatlas.cern.ch mkdir /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_31/HIGG5D2_13TeV/CxAOD_31-01_a/sigDBL/$F && xrdcp -r $F $E/sigDBL/$F
# copy from eos to your folder
# F=group.phys-higgs.mc16_13TeV.361030.CAOD_HIGG5D2.e3569_s3126_r10201_r10210_p3371.31-01_CxAOD.root
# cd $F
# in eos find out where is this DSID located
# find . -name "*361030*"
# you get ./MJ_Py8/group.phys-higgs.mc16_13TeV.361030.CAOD_HIGG5D2.e3569_s3126_r10201_r10210_p3371.31-01_CxAOD.root
# you copy (ony the files not already present) with
# xrdcp -r $E/MJ_Py8/$F . 
# alias to checking the non 1.000 samples
# alias g="grep -v 1.000 dslist_NevtDxAOD_yield.txt"
#############################

#############################
# parameters
#############################

# determines full output path (full_path)
energyTag     = "13TeV"
analysisFolder = derivationTag + "_" + energyTag + "/"
productionFolder = "CxAOD_" + productionTag + "/"
fullPath = destinationFolder + "/" + analysisFolder + productionFolder
if debug:
  print "energyTag",energyTag
  print "analysisFolder",analysisFolder
  print "productionFolder",productionFolder
  print "fullPath",fullPath

# determine isEOS
dir_path = os.path.dirname(os.path.realpath(__file__))
if debug:
  print "dir_path",dir_path
if dir_path.startswith("/eos"):
  # we copy from eos (and we can only copy to eos)
  # copy from local eos to local eos, so use the cp and the symbolic links
  isEOS=False
else:
  # we copy from another folder not in eos
  # but we have a choice to copy to that machine, or to eos
  # so we look at the destination folder
  isEOS = ("/eos/" in destinationFolder)
if debug:
  print "isEOS",isEOS

#############################
# sample definition
#############################

dict_DSID_folderName={}
with open("sample_info.txt") as f:
  content = f.read().splitlines()
  for line in content:
    list_line=line.split()
    dict_DSID_folderName[list_line[0]]=list_line[1]
# done loop over file of sample_info (dsid vs folderName)

def get_folderName(outDS,debug):
  if debug:
    print "get_folderName("+outDS+"):"
  list_outDSElement=outDS.split(".")
  if debug:
    print "len(list_outDSElement)=",len(list_outDSElement)
  if len(list_outDSElement) != 8:
    folderName="unknown"
  else:
    sampleUser1      = list_outDSElement[0] # user or group
    sampleUser2      = list_outDSElement[1] # abuzatu or phys-higgs
    sampleType       = list_outDSElement[2] # data17_13TeV or mc16_13TeV
    sampleRun        = list_outDSElement[3] # 00327490 or 363358
    sampleDerivation = list_outDSElement[4] # CAOD_HIGG5D1
    sampleAMITag     = list_outDSElement[5] # f838_m1824_p3372 or e6004_e5984_s3126_r9781_r9778_p3374
    sampleVTag       = list_outDSElement[6] # 30-02-3_CxAOD
    sampleExtension  = list_outDSElement[7] # root
    if debug:
      print('  {0:20s}{1:20s}{2:40s}'.format('internal variable', 'alias', 'value'))
      print('  {0:20s}{1:20s}{2:40s}'.format('sampleUser1', '', sampleUser1))
      print('  {0:20s}{1:20s}{2:40s}'.format('sampleUser2', '', sampleUser2))
      print('  {0:20s}{1:20s}{2:40s}'.format('sampleType', '', sampleType))
      print('  {0:20s}{1:20s}{2:40s}'.format('sampleRun', 'mc_channel_number', sampleRun))
      print('  {0:20s}{1:20s}{2:40s}'.format('sampleDerivation', '', sampleDerivation))
      print('  {0:20s}{1:20s}{2:40s}'.format('sampleAMITag', 'sample_ami', sampleAMITag))
      print('  {0:20s}{1:20s}{2:40s}'.format('sampleVTag', '', sampleVTag))
      print('  {0:20s}{1:20s}{2:40s}'.format('sampleExtension', '', sampleExtension))
    # done if
    sampleTypeWithoutEnergy=sampleType.split("_")[0]
    if "data" in sampleTypeWithoutEnergy:
      folderName=sampleTypeWithoutEnergy
    elif "mc" in sampleTypeWithoutEnergy:
      if sampleRun in dict_DSID_folderName.keys():
        folderName=dict_DSID_folderName[sampleRun]
      else:
        folderName="unknown"
    else:
      print "sampleTypeWithoutEnergy",sampleTypeWithoutEnergy,"for outDS",outDS,"so also say unknown"
      folderName="unknown"
    # done if data or mc
  # done if length of list 
  return folderName
# done function

#############################
# code
#############################

def makeDir(path):
  if (isEOS) : command = "xrd eosatlas.cern.ch mkdir " + path
  else :     command = "mkdir -p " + path
  if doVerbose : print command
  if not doTestOnly : os.system(command)

if __name__ == "__main__":

  print "Going to copy files from current directory to", fullPath

  # make sample dir
  makeDir(fullPath)
  if doCreateAllFoldersEvenIfYouDoNotHaveSamplesDownloaded:
    for DSID in dict_DSID_folderName:
      makeDir(fullPath + folderName+"/")

  # loop over subdirs in current dir
  for subdir, dirs, files in os.walk(".") :
    if debug:
      print "subdir",subdir
      print "dirs",dirs
      print "files", files
    if doVerbose : print "Directory:", subdir
    # determine sample
    # from subdir remove the "./" from the beginning
    outDS=subdir.replace("./","")
    sample=get_folderName(outDS,debug)
    if debug:
      print "outDS",outDS
      print "sample",sample
    if sample is "unknown":
      print "Warning: could not determine sample from directory'" + subdir + "'. Skipping."
      continue
    if doVerbose : print "Sample:", sample

    finalPath = fullPath + sample + "/" + subdir + "/"
    makeDir(finalPath)

    # loop over files in current subdir and copy
    for file in files :
      #file_path = subdir + "/" + file
      file_path = os.path.abspath(os.path.join(subdir,file))
      if doVerbose : print "File:", file
      if (isEOS) :
        command = "xrdcp "
        if not doSkipExisting : command += "--force "
        command += file_path + " root://eosatlas.cern.ch/" + finalPath + file
      else :
        if not doMakeSymbolicLink:
          command = "cp "
          if doSkipExisting : command += "-u "
          command += file_path + " "+ finalPath
        else :          
          newfile=file
          fixTChainName=True # deal with TChain feature by linking files of form  dir.root/file.root.X to dir.root/file.X.root
          if fixTChainName:
            print 'file_path',file_path
            print 'finalPath',finalPath
            lastroot = file.rfind(".root")
            filelen=len(file)
            if filelen>lastroot+5:
              lastsplit=file.split(".")
              tag=lastsplit[len(lastsplit)-1]
              newfile=newfile.replace(file[lastroot:filelen],"."+tag+".root")
          command = "ln -s "
          command += file_path + " "+ finalPath+newfile
      if doTestOnly :
        if doVerbose : print command
      else :         os.system(command)
    # done loop over files
  # done loop over folders

  # copy the auxiliary scripts and files, they will overwrite those already in eos
  if isEOS:
    command = "xrdcp --force"
  else:
    command = "cp"
  command+=" count_Nentry_SumOfWeight.py dslist_NevtDxAOD*.txt out_sample_list_sample_grid.*.txt checkYields.py yields.13TeV*_sorted.txt"
  if isEOS:
    command+=" root://eosatlas.cern.ch/"
  else:
    command+=" "
  command+=fullPath+"/."
  if doVerbose:
    print "command="+command
  os.system(command)

  # all done
  print ""
  print ""
  print "All finished fine!"

  
