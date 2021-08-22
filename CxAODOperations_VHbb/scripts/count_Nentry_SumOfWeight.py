#!/usr/bin/env python
# Expanded by Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAOD framework for VHbb analysis

import argparse
import collections
import hashlib
import math
import os
import ROOT
import string
import sys

from operator import itemgetter

ROOT.gROOT.SetBatch(True)

"""
Script for extracting initial number of events and sum of weights
from CxAODs, created from xAODs or DxAODs.
The current run directory is searched for the CxAODs.
Alternatively a out_sample_list can be given.

Treatment of different datasets with the same datasetid but different AMI tags (e.g. AFII, extensions etc.):
 Default behaviour:
  When creating standard yields file all datasets with the same dataset id will be summed
  When comparing with AMI can search and write extra tags to distinguish different
  versions of the same datasetid (see sample_ami)
"""

# this script runs for version 2.7 or higher
if not sys.version_info >= (2,7):
  print('ERROR: This script runs for python version 2.7 or higher.')
  sys.exit(1)

parser = argparse.ArgumentParser(
  description='''A script to count the number of events in CxAODs/DxAODs.''')
parser.add_argument('countNumberInDxAOD', metavar='countNumberInDxAOD',
                    type=int, choices=[0,1],
                    help='1: Check DxAOD yields. 0: Check CxAOD yields.')
parser.add_argument('validateLargeFiles', metavar='validateLargeFiles',
                    type=int, choices=[0,1],
                    help='Validate the CollectionTree in large files.'
                    ' Possible choices are 0/1.')
parser.add_argument('doSummed', metavar='doSummed',
                    type=int, choices=[0,1],
                    help='If for a given DSID there are several AMI tags, '
                    'add all together if 1; if 0, keep each as separate file.'
                    ' Possible choices are 0/1.')
parser.add_argument('-l', '--list', type=str, required=False, default=None,
                    dest='out_sample_list',
                    help='Consider only files that are in this list.')
parser.add_argument('-s', '--safe', action='store_true',
                    help='Fail on errors and some warnings (safe mode).')
parser.add_argument('-v', '--verbose', action='store_true',
                    help='Verbose mode.')
example_usage='''
example usage:
  {0} 1 0 0 # to compare with AMI
  {0} 0 0 1 # to run the Reader'''.format(__file__)
try:
  args = parser.parse_args()
except SystemExit:
  print example_usage
  sys.exit(1)

command = ' '.join(map(str, sys.argv))

print 'You ran: ' + command

# inputs
countNumberInDxAOD = args.countNumberInDxAOD
validateLargeFiles = args.validateLargeFiles
doSummed           = args.doSummed

# write the DxAOD andCxAOD yields to separate files all in one go
writeDxAODAndCxAOD=False
# do the checking of the root tree (somethimes useful to switch off during download etc.)
checkTree=True

debug = args.verbose

out_file_md5 = 'undefined'
invalid_files = []

def registerInvalidFile(message, f):
  print('ERROR: {}. Skipping file'.format(message, f))
  global invalid_files
  invalid_files += [f]

def main(argv):
  out_sample_list = args.out_sample_list
  if not out_sample_list:
    print('No sample list specified. Going to scan current directory for root '
          'files and then try to extract yields...')
  else:
    print("Trying to extract yields from files listed in '{}'"
          .format(out_sample_list))

  # check for valid file list file
  if out_sample_list and not os.path.isfile(out_sample_list):
    print("ERROR: File '{}' does not exist! Exiting.".format(out_sample_list))
    sys.exit(6)

  # List with allowed energies in file names
  nrj = [
    '13TeV',
    ]
  # dictionaries for steered sum, CxAOD and DxAOD
  counts = {}
  countsCxAOD = {}
  countsDxAOD = {}
  # prepare counter
  for i_nrj in nrj :
    counts[i_nrj] = []
    countsCxAOD[i_nrj] = []
    countsDxAOD[i_nrj] = []

  print('Prepared counters for all energies:')
  print('  --> {}'.format(counts))

  # in safe mode check that target yield files do not already exist
  if args.safe:
    for e in nrj:
      if countNumberInDxAOD:
        out_file_name = 'yields.{}_DxAOD_sorted.txt'.format(e)
      else:
        out_file_name = 'yields.{}_sorted.txt'.format(e)
      if os.path.isfile(out_file_name):
        print("ERROR: Target file '{}' exists already. Exiting"
              .format(out_file_name))
        sys.exit(7)

  # check for valid output directory
  # yieldDir = "./FrameworkSub/data/"
  yieldDir = './'
  copyFilesMD5 = False
  if out_sample_list:
    if os.path.isdir(yieldDir):
      copyFilesMD5 = True
    else:
      print("WARNING: Directory '{}' does not exist, can't copy output files!"
            .format(yieldDir))
      if args.safe:
        sys.exit(11)

  if debug:
    print 'copyFilesMD5',copyFilesMD5

  # copy file by appending its md5sum (first 10 characters) to the name
  md5sum = ''
  global out_file_md5
  if copyFilesMD5:
    md5sum = hashlib.md5(open(out_sample_list, 'rb').read()).hexdigest()
    if len(md5sum) > 10:
      md5sum = md5sum[:10]
    out_file_md5 = os.path.join(yieldDir, 'yields.txt.{}'.format(md5sum))
    if os.path.isfile(out_file_md5):
      print("File '{}' exists already, skipping yield counting."
            .format(out_file_md5))
      return

  if debug:
    print 'out_file_md5',out_file_md5, 'out_sample_list',out_sample_list

  if debug:
    print
    print '#############################################################'
    print '#### Create list of folders available #######################'
    print '#############################################################'
  list_folder = []
  if out_sample_list:
    with open(out_sample_list) as folders:
      print('Opened file {} of type {}'.format(folders, type(folders)))
      for folder in folders:
        folder=folder.rstrip() # removing \n at the end of the line
        folder+='_CxAOD.root'
        if debug:
          print('Scanning folder {} of type {}'.format(folder,type(folder)))
        list_folder.append(folder)
  if debug:
    print 'list_folder',list_folder

  if debug:
    print
    print '#############################################################'
    print '#### Compile file list by walking through the folders #######'
    print '#############################################################'
  # compile file list
  fileList = []
  for subdir, dirs, files in os.walk('.', followlinks=True):
    if debug:
      print('Content of {}'.format(subdir))
      if not dirs:
        print('  No subdirectories.')
      else:
        print('  Directories:')
        for d in dirs:
          print('    {}'.format(d))
      if not files:
        print('  No files.')
      else:
        print('  Files:')
        for f in files:
          print('    {}'.format(f))
    if out_sample_list: # then we want to skip folders that are not in the list
      myfolder = subdir.split('/')[-1]
      if not files or not myfolder in list_folder:
        continue
    # add files to the list
    for file in files :
      fileName = os.path.join(subdir, file)
      if debug:
        print('  --> Register file {}'.format(fileName))
      fileList.append(fileName)

  if debug:
    print 'List of my files:'
    for my_file in fileList:
      print('  {}'.format(my_file))

  #return

  if debug:
    print
    print '#############################################################'
    print '#### Loop over the files in the list and count ##############'
    print '#############################################################'
  # loop over root files and count
  for my_file in fileList :
    # root file?
    if not '.root' in my_file :
      print('INFO: Skipping non-root file {}'.format(my_file))
      continue

    # skip partially downloaded files
    if '.root.part' in my_file :
      print('INFO: Skipping partially downloaded file'.format(my_file))
      continue

    # energy
    curr_nrj = 'unknown'
    for i_nrj in nrj :
      if i_nrj in my_file :
        curr_nrj = i_nrj
    if 'unknown' in curr_nrj :
      print('WARNING: Energy not defined, skipping file'.format(my_file))
      continue

    if debug:
      print
      print '#############################################################'
      print '#### New file Loop over the files in the list and count #####'
      print '#############################################################'

    # Adrian style
    list_fileElement=my_file.split('/')
    outDS=list_fileElement[-2]
    if debug:
      # e.g. data user.abuzatu.data17_13TeV.00327490.CAOD_HIGG5D1.f838_m1824_p3372.30-MA-12-1_CxAOD.root
      # e.g. MC   user.abuzatu.mc16_13TeV.345058.CAOD_HIGG5D1.e6004_e5984_s3126_r9781_r9778_p3374.30-MA-12-1_CxAOD.root
      print "outDS",outDS
    list_outDSElement=outDS.split(".")
    if list_outDSElement[-1].endswith("_hist"):
      if debug:
        print "Skipping dataset with cut flow histograms ending in _hist"
      continue
    if len(list_outDSElement) == 8:
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
    elif len(list_outDSElement) == 6:
      # for pipeline, example mc16_13TeV.410501.PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_HIGG5D2.e5458_s3126_r9364_r9315_p3371:
      # ['mc16_13TeV', '410501', 'PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad', 'deriv', 'DAOD_HIGG5D2', 'e5458_s3126_r9364_r9315_p3371']
      sampleType       =list_outDSElement[0] # data17_13TeV or mc16_13TeV
      sampleRun        =list_outDSElement[1] # 00327490 or 363358
      sampleName       =list_outDSElement[2] # PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad
      sampleDeriv      =list_outDSElement[3] # deriv
      sampleDerivation =list_outDSElement[4] # DAOD_HIGG5D1
      sampleAMITag     =list_outDSElement[5] # f838_m1824_p3372 or e6004_e5984_s3126_r9781_r9778_p3374
      if debug:
        print('  {0:20s}{1:20s}{2:40s}'.format('internal variable', 'alias', 'value'))
        print('  {0:20s}{1:20s}{2:40s}'.format('sampleType', '', sampleType))
        print('  {0:20s}{1:20s}{2:40s}'.format('sampleRun', 'mc_channel_number', sampleRun))
        print('  {0:20s}{1:20s}{2:40s}'.format('sampleName', '', sampleName))
        print('  {0:20s}{1:20s}{2:40s}'.format('sampleDeriv', '', sampleDeriv))
        print('  {0:20s}{1:20s}{2:40s}'.format('sampleDerivation', '', sampleDerivation))
        print('  {0:20s}{1:20s}{2:40s}'.format('sampleAMITag', 'sample_ami', sampleAMITag))
    else:
      print('ERROR: list_outDSElement is in wrong format.'
            ' Can only handle 6 ot 8 elements.')
      print('list_outDSElement = {}'.format(list_outDSElement))
      sys.exit(1)
    # done if

    mc_channel_number=sampleRun
    sample_ami=sampleAMITag

    def frmt(name, val):
      print('    {:28s}{}'.format(name, val))
    print '--> Processing information'
    frmt('File', my_file)
    frmt('Sample Derivation', sampleDerivation)
    frmt('Energy', curr_nrj)
    frmt('MC Channel Number', mc_channel_number)

    
    # read file and event count histogram
    file = ROOT.TFile.Open(my_file,'read')
    if not file:
      registerInvalidFile('Cannot read file!', my_file)
      continue
    if file.IsZombie():
      registerInvalidFile('Zombie file, probably broken!', my_file)
      continue
    h = file.Get('MetaData_EventCount')
    if not h:
      registerInvalidFile('MetaData_EventCount not found!', my_file)
      continue

    # check for valid CollectionTree
    if file.GetSize() < 1e6 or validateLargeFiles:
      if debug:
        print '  Check for valid CollectionTree'
      tree = file.Get('CollectionTree')
      if not tree:
        registerInvalidFile('CollectionTree not found!', my_file)
        continue
      if checkTree:
        if tree.GetEntries() > 0 and tree.GetEntry(0) <= 0:
          registerInvalidFile('Invalid CollectionTree!', my_file)
          continue
        
    # count from DxAOD metadata
    if debug:
      print '  Count from DxAOD metadata'
    n_entries = h.GetBinContent(1)
    n_selout_entries = h.GetBinContent(3)
    if countNumberInDxAOD:
      n_selout_entries = h.GetBinContent(2)
#
    n_selout_entriesDxAOD = h.GetBinContent(2)
    n_selout_entriesCxAOD = h.GetBinContent(3)

    n_sum = h.GetBinContent(4)

    if n_entries == 0 :
      print '  WARNING: Zero AOD events in MetaData for file {}'.format(my_file)
      if args.safe:
        file.Close()
        sys.exit(11)

    if countNumberInDxAOD and n_selout_entries == 0 :
      print '  WARNING: Zero DxAOD events in MetaData for file {}'.format(my_file)
      if args.safe:
        file.Close()
        sys.exit(11)
    elif n_selout_entriesDxAOD == 0:
      print '  WARNING: Zero DxAOD events in MetaData for file {}'.format(my_file)
      if args.safe:
        file.Close()
        sys.exit(11)
      
    file.Close()

    frmt('Number of Entries (AOD)', n_entries)
    if writeDxAODAndCxAOD:
      frmt('Selected Entries (DxAOD)', n_selout_entriesDxAOD)
      frmt('Selected Entries (CxAOD)', n_selout_entriesCxAOD)
    elif countNumberInDxAOD:
      frmt('Selected Entries (DxAOD)', n_selout_entries)
    else:
      frmt('Selected Entries (CxAOD)', n_selout_entries)
    frmt('Sum (AOD)', n_sum)
    frmt('Sample AMI', sample_ami)

    # find counter based on the mc_channel_number and sample_ami
    if debug:
      print 'This is the counts dictionary:'
      for key, count_lists in counts.iteritems():
        print('  key = {}:'.format(key))
        print('  {} count lists available:'.format(len(count_lists)))
        for count in count_lists:
          print('    {}'.format(count))
      #
      print 'This is the counts CxAOD dictionary:'
      for key, count_lists in countsCxAOD.iteritems():
        print('  key = {}:'.format(key))
        print('  {} count lists available:'.format(len(count_lists)))
        for count in count_lists:
          print('    {}'.format(count))
      #
      print 'This is the counts DxAOD dictionary:'
      for key, count_lists in countsDxAOD.iteritems():
        print('  key = {}:'.format(key))
        print('  {} count lists available:'.format(len(count_lists)))
        for count in count_lists:
          print('    {}'.format(count))

#
    if writeDxAODAndCxAOD:
      countCxAOD = countsCxAOD[curr_nrj]
      countDxAOD = countsDxAOD[curr_nrj]
    else:
      count = counts[curr_nrj]  
# 
    if writeDxAODAndCxAOD:
      counterCxAOD = ['unknown', 0, 0, 0]
      counterDxAOD = ['unknown', 0, 0, 0, 'unknown_sample_ami']
    else:  
      if doSummed:
        counter = ['unknown', 0, 0, 0]
      else:
        counter = ['unknown', 0, 0, 0, 'unknown_sample_ami']

#
    if writeDxAODAndCxAOD:
      for i_count in countCxAOD :
        if mc_channel_number == i_count[0]:
          counterCxAOD = i_count
      for i_count in countDxAOD :
          if mc_channel_number == i_count[0] and sample_ami == i_count[4]:
            counterDxAOD = i_count 
    else:
      for i_count in count :
        if doSummed:
          if mc_channel_number == i_count[0]:
            counter = i_count
        else:
          if mc_channel_number == i_count[0] and sample_ami == i_count[4]:
            counter = i_count
    #
   
    # this takes care of the first file where count is empty
    if writeDxAODAndCxAOD:
      if 'unknown' in counterCxAOD[0]:
        counterCxAOD = [mc_channel_number, 0, 0, 0]
        if debug:
          print 'appending CxAOD',counterCxAOD
        countCxAOD.append(counterCxAOD)
      if 'unknown' in counterDxAOD[0]:
        counterDxAOD = [mc_channel_number, 0, 0, 0, sample_ami]
        if debug:
          print 'appending DxAOD',counterDxAOD
        countDxAOD.append(counterDxAOD)
#
    else:
      if 'unknown' in counter[0]:
        if doSummed:
          counter = [mc_channel_number, 0, 0, 0]
        else:
          counter = [mc_channel_number, 0, 0, 0, sample_ami]
        if debug:
          print 'appending',counter
        count.append(counter)


    if writeDxAODAndCxAOD:
      counterCxAOD[1] += n_entries
      counterDxAOD[1] += n_entries
      counterCxAOD[2] += n_selout_entriesCxAOD
      counterDxAOD[2] += n_selout_entriesDxAOD
      counterCxAOD[3] += n_sum
      counterDxAOD[3] += n_sum
    else:
      counter[1] += n_entries
      counter[2] += n_selout_entries
      counter[3] += n_sum  
      
  # done loop over files
  if debug:
    print
    print '#############################################################'
    print '#### Done Loop over the files in the list and count #########'
    print '#############################################################'

  if len(invalid_files) > 0:
    print '\nERROR: Found invalid files! Exiting. You can delete the files with:\n'
    print 'rm \\'
    for fileName in invalid_files:
      print fileName, '\\'
    print '** NO YIELD FILE WAS PRODUCED **'
    sys.exit(1)

  # write list
  if writeDxAODAndCxAOD:
    for i_nrj in nrj :
      # 
      countCxAOD = countsCxAOD[i_nrj]
      if len(countCxAOD) is 0:
        print 'INFO: No yields found for ' + i_nrj + '. Will write an empty CxAOD file.'
      # 
      countDxAOD = countsDxAOD[i_nrj]
      if len(countDxAOD) is 0:
        print 'INFO: No yields found for ' + i_nrj + '. Will write an empty DxAOD file.'
      # sort by mc_channel_number
      countCxAOD = sorted(countCxAOD, key=itemgetter(0))
      countDxAOD = sorted(countDxAOD, key=itemgetter(0))
      # define file
      out_file_nameCxAOD = './yields.'+i_nrj+'_sorted.txt'
      out_file_nameDxAOD = './yields.'+i_nrj+'_DxAOD_sorted.txt'
      print 'Writing CxAOD', out_file_nameCxAOD
      print 'Writing DxAOD', out_file_nameDxAOD
      out_fileCxAOD = open(out_file_nameCxAOD, 'w')
      out_fileDxAOD = open(out_file_nameDxAOD, 'w')
      # write
      for i_count in countCxAOD :
        lineCxAOD='%10s %10.0f %10.0f %15f' % (i_count[0], i_count[1], i_count[2], i_count[3])
        out_fileCxAOD.write(lineCxAOD+'\n')
      out_fileCxAOD.close()
      for i_count in countDxAOD :
        lineDxAOD='%10s %10.0f %10.0f %15f %-30s' % (i_count[0], i_count[1], i_count[2], i_count[3], i_count[4])
        out_fileDxAOD.write(lineDxAOD+'\n')
      out_fileDxAOD.close()
  else :  
    for i_nrj in nrj :
      # pick the good table
      count = counts[i_nrj]
      if len(count) is 0:
        print 'INFO: No yields found for ' + i_nrj + '. Will write an empty file.'
      # sort by mc_channel_number
      count = sorted(count, key=itemgetter(0))
      # define file
      extrafiletag = ''
      if countNumberInDxAOD:
        extrafiletag = '_DxAOD'
      out_file_name = './yields.'+i_nrj+extrafiletag+'_sorted.txt'
      print 'Writing', out_file_name
      out_file = open(out_file_name, 'w')
      # write
      for i_count in count :
        if doSummed:
          line='%10s %10.0f %10.0f %15f' % (i_count[0], i_count[1], i_count[2], i_count[3])
        else:
          line='%10s %10.0f %10.0f %15f %-30s' % (i_count[0], i_count[1], i_count[2], i_count[3], i_count[4])
        out_file.write(line+'\n')
      out_file.close()

  # copy yield file by appending the md5sum of input files
  if copyFilesMD5:
    # check which energy
    # TODO should energy be removed from this script completely?
    curr_nrj = ''
    for i_nrj in nrj :
      if len(counts[i_nrj]) > 0:
        if curr_nrj != '':
          print "ERROR: Found multiple energies in file list, can't copy output file!"
          return
        curr_nrj = i_nrj
    if curr_nrj is '':
      return
    out_file_sort = './yields.'+curr_nrj+'_sorted.txt'
    print 'Copying file '' + out_file_sort + '' to '' + out_file_md5 + ''...'
    os.system('cp ' + out_file_sort + ' ' + out_file_md5)

    out_sample_list_md5 = yieldDir + out_sample_list + '.' + md5sum
    print 'Copying file '' + out_sample_list + '' to '' + out_sample_list_md5 + ''...'
    os.system('cp ' + out_sample_list + ' ' + out_sample_list_md5)

if __name__ == '__main__':
    main(sys.argv[1:])
    # Correct sum of weights to fix the high MC weights issue seen on ggZH samples - To be removed when the issue is fixed
    # If the eos / local cluster repository where count_Nentry_SumOfWeight.py is stored is affected by the ggZH bug (namely MC16d and MC16e)
    # please enable the affectedRepository = True in correct_SumOfWeight.py 
    file_path = os.path.dirname(__file__)
    if os.path.isfile(file_path + 'correct_SumOfWeight.py'):
        log = os.popen('python %s'%(file_path + 'correct_SumOfWeight.py')).read()
        print log
