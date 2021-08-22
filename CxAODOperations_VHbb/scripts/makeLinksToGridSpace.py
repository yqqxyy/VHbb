#!/usr/bin/env python
# Make links from eos to the files on CERN grid space using the standard CxAOD directory structure
#
# useful to be able to read/write dictionaries easily so get print from the future
from __future__ import print_function
import os, sys
import argparse
import subprocess

dict_dataset_grid={}
def fillSampleMap(infile):
  # use rucio to see if datasets from infile are stored at CERN grid and fill map with eos grid path 
  f=open(infile)
  content = f.read().splitlines()
  for line in content:
    list_line=line
    if doVerbose:
      print('fillSampleMap',list_line)
    # check for empty lines
    if not list_line:
      continue
    dataset=list_line
    dataset+="_CxAOD.root"
    if doVerbose:
      print(list_line,dataset)
    #
    try:
      proc = subprocess.Popen(['rucio', 'list-file-replicas', '--rse', 'CERN-PROD_PHYS-HIGGS', dataset],
                              stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    except OSError:
      print('rucio is not configured. Cannot continue.')
      print('Did you forget to lsetup rucio?')
      sys.exit(1)

    isErr=False
    for line in proc.stderr:
      isErr=True
    if doVerbose:
      print('isErr:',isErr)
    #  skip if there is an error - usually means dataset is not existing - check the dataset lists 
    if isErr:
      print("rucio error (serious) for dataset",dataset)
      continue
      
    filelist=[]
    #for rucioline in out.readline():
    for rucioline in proc.stdout:
      startString='/eos/atlas/'
      startpos=rucioline.find(startString)
      if doVerbose:
        print('rucioline:',rucioline,'start position:',startpos)
      if startpos>=0:        
        endString='CxAOD.root'          
        endpos=rucioline.rfind(endString)
        eospath=rucioline[startpos:endpos+len(endString)]
        if doVerbose:
          print('appending eospath',eospath,'for',dataset)
        filelist.append(eospath)
        
    # fill map for replicated datasets
    if len(filelist)>0:
      dict_dataset_grid[dataset]=filelist
      # possible checks:
      # rucio list-files --csv dataset gives number of rows equal to nfiles
      # compare to size of filelist, if don't agree do an --rse print out with --missing to flag
    else:
      print("rucio error (all files missing at CERN) for dataset",dataset)

  # end of loop  over datasets    
  if (doVerbose):    
    print('dataset_eos map',dict_dataset_grid)


def makeDir(path):
  if (isEOS): 
    command = "xrdfs eosatlas.cern.ch mkdir -p " + path
  else :     
    command = "mkdir -p " + path
  if doVerbose: 
    print(command)
  if not doTestOnly: 
    os.system(command)

def get_folderName(outDS,debug):
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


dict_DSID_folderName={}
def fillSampleInfo(file):
  # fill the map of dataset id to the sample directory from the sample info file
  with open(file) as f:
    content = f.read().splitlines()
    for line in content:
      list_line=line.split()
      dict_DSID_folderName[list_line[0]]=list_line[1]
# done loop over file of sample_info (dsid vs folderName)


if __name__ == "__main__":

  parser = argparse.ArgumentParser()     
  parser.add_argument('indataset', help='input dataset file')     
  parser.add_argument('outdirectory', help='output directory')     
  parser.add_argument('-s', '--sampleinfo', help='sample datasetid-directory info file', default='sample_info.txt')
  parser.add_argument('-t', '--test', action='store_true',help='test mode')
  parser.add_argument('-w', '--writemap', help='write the dataset-eospath map to a file')
  parser.add_argument('-r', '--readmap', help='read the dataset-eospath map from a file and just do linking')
  parser.add_argument('-d', '--debug', action='store_true',help='debugging mode')
  parser.add_argument('-v', '--verbose', action='store_true', help='verbose mode')     
  args = parser.parse_args()     
  print(parser.parse_args())

  datasetin=args.indataset  
  fullPath=args.outdirectory
  sample_info=args.sampleinfo
  doTestOnly=args.test
  mapOut=args.writemap
  mapIn=args.readmap
  debug=args.debug
  doVerbose=args.verbose

  # output directory
  if debug:
    print("fullPath",fullPath)

  isEOS = ("/eos/" in fullPath)
  if debug:
    print("isEOS",isEOS)

  # loop over the dataset input file and get eospath at CERN, unless a filemap is provided
  if mapIn == None:
    fillSampleMap(datasetin)
  else:
    with open(mapIn, 'r') as f:
      print("reading map from file")
      mapin = f.read()
      dict_dataset_grid=eval(mapin)
      print(dict_dataset_grid)

  if mapOut != None:
    print('mapout',mapOut)
    with open(mapOut, 'w') as f:
      print(f)
      print(dict_dataset_grid, file=f)

  # fill the map of dsid-folder as given in sample_info file
  fillSampleInfo(sample_info)
  if doVerbose:    
    print("Sample info file:",sample_info,"with map",dict_DSID_folderName)

  print("Going to copy files from current directory to", fullPath)

  # make sample dir
  makeDir(fullPath)

  # loop over dataset-eos grid path map
  if debug or doVerbose:
    print('dataset eos map:',dict_dataset_grid)
  for dataset,files in dict_dataset_grid.iteritems():
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

    # loop over files in current subdir and copy
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






  
