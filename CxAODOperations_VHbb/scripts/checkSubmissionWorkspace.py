#!/usr/bin/env python

# Written by Andreas Hoenle (der.andi@cern.ch)
# for the benefit of the ATLAS collaboration

"""
This script performs a number of checks before submitting a CxAOD production

Current
  * Is the WorkSpace configured? (limited number of checks)
  * Is a download destination for every DSID about to submitted defined?
  * Are there DSIDs about to be submitted that are not in the X-section file?
"""

import argparse
import os
import sys

from socket import gethostname

debug=False

parser = argparse.ArgumentParser(
    description='Describe me a bit')
parser.add_argument('-a', '--all', action='store_true',
                    help="Don't ignore comments (lines starting with '#' "
                    "in ./*.txt)")

args = parser.parse_args()

string_format_width = 18
sfw = string_format_width

def main():
    if not sys.version_info >= (2, 7):
        print('You need at least python version 2.7 (forgot to setup athena?)')
        sys.exit(1)

    if not check_workspace():
        print('Error: Invalid workspace')
        sys.exit(2)
    print('Success: Workspace seems to be okay.')
    print
    if not check_sample_info_file():
        print('Error: The script found problems with the sample info file')
        sys.exit(4)
    print('Success: Syntax correct in sample info file')
    print
    if not check_download_destinations_and_xsectionfile():
        print('Error: The script found problems. Please check the Errors above.')
        sys.exit(3)
    print('Success: Valid configuration')
    print('Success: Everything appears to be in order :)')

def get_operations_vhbb_path():
    operations_vhbb_path = os.path.join(os.getenv('WorkDir_DIR'), '..', '..', 'source',
                         'CxAODOperations_VHbb')
    if not os.path.isdir(operations_vhbb_path):
        print('Error: Could not find CxAODOperations_VHbb here: {}'
              .format(operations_vhbb_path))
        return None
    return operations_vhbb_path

def get_operations_path():
    operations_path = os.path.join(os.getenv('WorkDir_DIR'), '..', '..', 'source',
                         'CxAODOperations')
    if not os.path.isdir(operations_path):
        print('Error: Could not find CxAODOperations_VHbb here: {}'
              .format(operations_path))
        return None
    return operations_path

def get_sample_info_file():
    operations_vhbb_path = get_operations_vhbb_path()
    if not operations_vhbb_path:
        return None
    # find the list with the target samplenames
    sample_info_file = os.path.join(operations_vhbb_path, 'data', 'DxAOD', 'info', 'sample_info.txt')
    if not os.path.isfile(sample_info_file):
        print('Error: Could not find sample info file here: {}'
              .format(sample_info_file))
        return None
    return sample_info_file


def check_sample_info_file():
    sample_info_file = get_sample_info_file()
    if not sample_info_file:
        return False
    # check that it follows the updated syntax (https://gitlab.cern.ch/CxAODFramework/FrameworkSub/issues/28)
    with open(sample_info_file) as sif:
        for i, line in enumerate(sif.readlines()):
            lno = i + 1
            if not line or line.isspace():
                # empty line
                continue
            if line.startswith('#'):
                # ignore comments, starting with '#'
                continue
            split = line.split()
            if len(split) < 4:
                print('Error in line {} of {} (too few values)'
                      .format(lno, sample_info_file))
                print('  Every line must contain')
                print('  <DSID> <short_name> <full_name> <is_nominal>')
                print("  'veto' can be specified optionally in the fifth column")
                print('  Find details here: https://gitlab.cern.ch/CxAODFramework/FrameworkSub/issues/28')
                return False
            try:
                int(split[0])
            except:
                print('Error in line {} of {}'
                      .format(lno, sample_info_file))
                print('  Cannot cast first column to integer')
                return False
            if not (split[3] == 'nominal' or split[3] == 'alternative'):
                print('Error in line {} of {}'
                      .format(lno, sample_info_file))
                print("  Third column must specify 'nominal' or 'alternative'."
                      " I found '{}'".format(split[3]))
                return False
            if len(split) > 4 and split[4] != 'veto':
                print('Error in line {} of {}'
                      .format(lno, sample_info_file))
                print("  Can only specify 'veto' in fifth column. "
                      "You gave me '{}'".format(split[4]))
                return False
            if len(split) > 5:
                print('Warning: Unexpected additional values in line {} of file {}'
                      .format(lno, sample_info_file))
    return True


def check_workspace():
    """Check the workspace.

    This function has *limited* functionality, it only checks if a workspace
    is set
    """

    alrbtools = os.getenv('ALRB_availableTools')
    if not alrbtools or not 'asetup' in alrbtools:
        print('Cannot find asetup. Did you do setupATLAS?')
        return False

    version = os.getenv('AtlasVersion')
    if not version:
        print('AtlasVersion is not set. Did you do asetup?')
        return False
    print('{0:{width}s} = {1}'.format('AtlasVersion', version, width=sfw))

    workdir = os.getenv('WorkDir_DIR')
    if not workdir:
        print('WorkDir_DIR is not set. Did you source the '
              'build/x86_64-slc6-gcc62-opt/setup.sh ?')
        return False
    if not os.path.isdir(workdir):
        print("WorkDir_DIR ('{}') is not a directory".format(workdir))
        return False
    print('{0:{width}s} = {1}'.format('WorkDir', workdir, width=sfw))

    framework_release_file = os.path.join(workdir, '..', '..', 'source',
                                 'CxAODBootstrap_VHbb', 'bootstrap', 'release.txt')
    if not os.path.isfile(framework_release_file):
        print('Error: Cannot find the framework release file here: {}'
              .format(framework_release_file))
        return False
    with open(framework_release_file) as f:
        framework_release = f.readlines()[0].rstrip('\n')  # first line w/o \n
    if framework_release != version:
        print('Error: Framework release ({}) does not matches the local setup ({})'
              .format(framework_release, version))
        return False
    print('{0:{width}s} = {1}'
          .format('Framework Release', framework_release, width=sfw))

    if not 'lxplus' in gethostname():
        print("Warning: You're not on lxplus. Take extra caution, "
              "as the HowTo on TWiki is tested on lxplus only.")
    return True


def check_download_destinations_and_xsectionfile():
    """
    1)
      Check if there are DSIDs in the current folder ./*.txt that are not
       present in CxAODOperations_VHbb/data/DxAOD/info/sample_info.txt
    2)
      Check if all DSIDs are also in the cross section file
    """

    operations_path = get_operations_path()
    if debug:
        print "operations_path",operations_path
    if not operations_path:
        return False

    infiles_dir = os.path.join('./') # the current folder
    if debug:
        print "infiles_dir",infiles_dir

    infiles = os.listdir(infiles_dir)
    if debug:
        print "infiles",infiles

    # keep only .txt files that start with list_sample_grid.mc16, prepend the path
    infiles = [os.path.join(infiles_dir, f)
               for f in infiles if f.endswith('.txt') and "list_sample_grid.13TeV_25ns." in f]
    # make list with all dsids in the files
    if debug:
        print "infiles",infiles
    all_dsids = {}
    for f in infiles:
        with open(f) as infile:
            for line in infile.readlines():
                line=line.rstrip()
                if line.startswith('#'):
                    # comment
                    if not args.all:
                        continue
                    while line and line.startswith('#'):
                        line = line.lstrip('#')
                    line += ' (commented out)'
                if line.startswith('data'):
                    # skip data
                    continue
                if line=="":
                    # skip empty line
                    continue
                list_line=line.split('.')
                if len(list_line)==0:
                    print "ERROR! no . in line",line
                    assert(False)
                dsid = line.split('.')[1]
                try:
                    dsid = int(dsid)
                except:
                    # skip filenames that are not in the expected format
                    pass
                else:
                    # save together will an array of all lines for that DSID
                    if dsid not in all_dsids:
                        all_dsids[dsid] = [line.rstrip('\n')]
                    else:
                        all_dsids[dsid].append(line.strip('\n'))

    if debug:
        print "all_dsids",type(all_dsids)
        print all_dsids
    all_good = True

    # extract dsids that have a target samplename
    dsids_with_target = []
    dsids_vetos = []
    sample_info_file = get_sample_info_file()
    if not sample_info_file:
        return False
    with open(sample_info_file) as sif:
        for line in sif.readlines():
            split = line.split()
            try:
                dsid = int(split[0])
            except:
                # skip lines where the first part is not an integer
                # (reported above)
                pass
            else:
                dsids_with_target.append(dsid)
                if len(split) > 4 and split[4] == 'veto':
                    dsids_vetos.append(dsid)
    print('I found {} unique MC DSIDs in the In/*.txt files. '
          'The sample_info.txt has {} entries'
          .format(len(all_dsids), len(dsids_with_target)))
    # print feedback and return False if there are problems
    missing_info = [dsid for dsid in sorted(all_dsids.keys())
                    if dsid not in dsids_with_target]
    if missing_info:
        print("Error: That's a mismatch")
        print('You have to specify destinations for the following DSIDs in the '
              'sample_info.txt:')
        for mis_dsid in missing_info:
            print('DSID: {}, Name: {}'
                  .format(mis_dsid, file_gist(all_dsids[mis_dsid][0])))
        print('\nA template of how to add these in the same style of sample_info.txt add at the top, '
              'and then sort the file so that they appear ordered by DSID:')
        for mis_dsid in missing_info:
            print "%-24s %-34s %-69s %-7s" % (mis_dsid, "eosFolder", file_gist(all_dsids[mis_dsid][0]), "nominal")
            #print('{}                   eosFolder                             {}           nominal'
            #      .format(mis_dsid, file_gist(all_dsids[mis_dsid][0])))
        all_good = False
        print

    # check if all the dsids are in the xsectionfile
    xsec_file = os.path.join(operations_path, 'data', 'XSections_13TeV.txt')
    if not os.path.isfile(xsec_file):
        print('Error: Could not find the cross section file here: {}'
              .format(xsec_file))
        return False
    dsids_in_xsec = []
    with open(xsec_file) as xs:
        for line in xs.readlines():
            if line.startswith('#'):
                continue
            try:
                dsid = int(line.split()[0])
            except:
                # skip lines with non-int first entry
                pass
            else:
                dsids_in_xsec.append(dsid)
    missing_xsec = [dsid
                    for dsid in sorted(all_dsids.keys())
                    if dsid not in dsids_in_xsec
                    and dsid not in dsids_vetos]
    # print feedback and return False if there are problems
    if missing_xsec:
        print('Error: Not all DSIDs are specified in the cross section file')
        print('The following DSIDs are missing:')
        for mis_dsid in missing_xsec:
            print('DSID: {}, Name: {}'
                  .format(mis_dsid, file_gist(all_dsids[mis_dsid][0])))
        all_good = False
        print

    return all_good

def file_gist(fname):
    """
    for a file like e.g. mc16_13TeV.304102.MadGraphPythia8EvtGen_A14NNPDF23LO_zp2hdm_bb_mzp600_mA300.deriv.DAOD_HIGG2D4.e4429_e5984_s3126_r9781_r9778_p3374
    we want to extract the relevant part: MadGraphPythia8EvtGen_A14NNPDF23LO_zp2hdm_bb_mzp600_mA300
    """
    return fname.split('.')[2]

if __name__ == '__main__':
    main()
