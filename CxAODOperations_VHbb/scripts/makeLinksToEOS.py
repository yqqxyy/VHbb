#!/usr/bin/env python
#
# Make links from eos to other eos directories containing the datasets using the standard CxAOD directory structure (as provided by sample_info)
# based on parts of the copy script
#
#
# useful to be able to read/write dictionaries easily so get print from the future
from __future__ import print_function
import os, sys
import argparse
import subprocess

dict_datasetname_eospath={} # map of grid dataset name to eos path
dict_DSID_folderName={} # map of dsid to folder name as given by sample_info.txt

def makeDir(path):
  # make the eos path
  if (isEOS): 
    command = "xrdfs eosatlas.cern.ch mkdir -p " + path
  else :     
    command = "mkdir -p " + path
  if doVerbose: 
    print(command)
  if not doTestOnly: 
    os.system(command)

def get_folderName(outDS,debug):
  # for the input dataset return the eos folder name as given by the sample_info map
  if debug:
    print("get_folderName("+outDS+"):")
  list_outDSElement=outDS.split(".")
  if debug:
    print("len(list_outDSElement)=",len(list_outDSElement))
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
      print("sampleTypeWithoutEnergy",sampleTypeWithoutEnergy,"for outDS",outDS,"so also say unknown")
      folderName="unknown"
    # done if data or mc
  # done if length of list 
  return folderName
# done function



def fillSampleInfo(file):
  # fill the map of dataset id to the sample directory from the sample info file
  with open(file) as f:
    content = f.read().splitlines()
    for line in content:
      list_line=line.split()
      dict_DSID_folderName[list_line[0]]=list_line[1]
# done loop over file of sample_info (dsid vs folderName)


if __name__ == "__main__":

  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                                     description="make links from outdirectory to the eos files listed in the text file")     

  parser.add_argument('filelist', help='text file containing full path of eos root files')
  parser.add_argument('outdirectory', help='output directory where the links will be made from (will be created if does not exist')     
  parser.add_argument('-s', '--sampleinfo', help='sample datasetid-directory info file', default='sample_info.txt')
  parser.add_argument('-t', '--test', action='store_true',help='test mode')
  parser.add_argument('-d', '--debug', action='store_true',help='debugging mode')
  parser.add_argument('-v', '--verbose', action='store_true', help='verbose mode')     
  args = parser.parse_args()     
  print(parser.parse_args())

  fileList=args.filelist
  fullPath=args.outdirectory
  sample_info=args.sampleinfo
  doTestOnly=args.test
  debug=args.debug
  doVerbose=args.verbose

  # output directory
  if debug:
    print("fullPath",fullPath)

  isEOS = ("/eos/" in fullPath)
  if debug:
    print("isEOS",isEOS)

  # loop over the list of root files and fill a map of datasetname to filepaths of root files 
  if fileList:
    # open the file and add file list to dataset map  
    with open(fileList, 'r') as f:
      print("reading dataset files from file (ready to fill map)")
      for line in f:
        line=line.strip('\n')
        #print(line)
        dataset='none'
        filelist=[]
        splitline=line.split("/")
        for split in splitline:
          if '_CxAOD.root' in split:
            dataset=split
        if dataset not in dict_datasetname_eospath:          
          dict_datasetname_eospath[dataset]=[line]
        else:
          dict_datasetname_eospath[dataset].append(line)

    #
    print('made the following dictionary of datasets to eospath:',dict_datasetname_eospath)
  else:
    print("no file list supplied",fileList)


  # fill the map of dsid-folder as given in sample_info file
  fillSampleInfo(sample_info)
  if doVerbose:    
    print("Sample info file:",sample_info,"with map",dict_DSID_folderName)

  print("Going to make links from", fullPath)

  # make sample dir if not existing
  makeDir(fullPath)

  # loop over dataset-eos grid path map
  if debug or doVerbose:
    print('dataset eos map:',dict_datasetname_eospath)
  for dataset,files in dict_datasetname_eospath.iteritems():
    if debug:
      print("dataset",dataset)
      print("files", files)
    # determine sample
    subdir=dataset
    outDS=dataset 
    sample=get_folderName(outDS,debug)
    if debug:
      print("outDS",outDS)
      print("sample",sample)
    if sample is "unknown":
      print("Warning: could not determine sample from directory'" + subdir + "'. Skipping.")
      continue
    if doVerbose : print("Sample:", sample)

    finalPath = fullPath + sample + "/" + subdir + "/"
    makeDir(finalPath)

    # loop over files in current subdir and make link
    for file in files :
      file_path = os.path.abspath(os.path.join(subdir,file))
      if doVerbose : print("File:", file,file_path)
      command = "ln -s "
      command += file_path + "  "+ finalPath+"."
      if doTestOnly :
        print(command)
      else :         
        if doVerbose : print(command)
        os.system(command)
      # done loop over files
  # done loop over folders

  # all done
  print("Done")






  
