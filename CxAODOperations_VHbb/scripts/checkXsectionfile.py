#!/usr/bin/env python

import argparse
import os
import sys
import textwrap

"""
Written by Andreas Hoenle (der.andi@cern.ch) 
for the benefit of the ATLAS collaboration.

The script checkXsectionfile.py validates the cross section file that is located
in CxAODOperations/data.
If the user wants to specify her own cross section file it can be done by running

  checkXsectionfile.py -f /path/to/custom/XSectionfile.txt

The script has two main tasks:
  1) Check if there are channels appearing twice in the file
  2) (optional) scan through all CxAODs in a specified path and check if they are
     covered in the XSectionfile.txt

The second option is activated by specifying a path as optional argument

  checkXsectionfile.py --cxaod /path/to/cxaods
"""

# This is the list with responsible people
# The user will be asked to contact the specified people in case of problems
responsibles = {
    # format: SAMPLE: 'John Doe <john.doe@cern.ch>'
    '3tops':                 '',
    'bbA':                   '',
    'bbH125':                'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'ggA':                   '',
    'ggH125_bb':             'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'ggH_inc':               'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'ggZH125':               'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'ggZH125_cc':            'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'ggZZ':                  'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'HVTWHlvbb':             '',
    'HVTZHllbb':             '',
    'HVTZHvvbb':             '',
    'qqZZ':                  '',
    'singletop_s':           '',
    'singletop_t':           '',
    'singletop_Wt':          '',
    'ttbar_allhad_Sh221':    '',
    'ttbar_dilep':           '',
    'ttbar_dilep_A14':       '',
    'ttbar_nonallhad':       '',
    'ttbar_nonallhad_A14':   '',
    'ttbar_nonallhad_A14_s': '',
    'ttH':                   '',
    'ttH_dilep':             '',
    'ttV':                   '',
    'VBF_inc':               'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'WenuB_v221':            'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WenuC_v221':            'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WenuHPTPTV_v221':       'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WenuHPT_v221':          'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WenuL_v221':            'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WH125J_MINLO_inc':      'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'WHlv125J_MINLO':        'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'WHlv125J_MINLO_old':    'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'WmumuHPTPTV_v221':      'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WmunuB_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WmunuC_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WmunuHPTPTV_v221':      'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WmunuHPT_v221':         'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WmunuL_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WtaunuB_v221':          'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WtaunuC_v221':          'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WtaunuHPTPTV_v221':     'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WtaunuHPT_v221':        'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WtaunuL_v221':          'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'WW':                    '',
    'WZ':                    '',
    'ZeeB_v221':             'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZeeC_v221':             'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZeeHPTPTV_v221':        'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZeeHPT_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZeeL_v221':             'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZH125J_MINLO':          'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'ZH125J_MINLO_llcc':     'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'ZH125J_MINLO_old':      'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'ZH125J_Zincl':          'Tatsuya Masubuchi <tatsuya.masubuchi@cern.ch> and Valerio Dao <valerio.dao@cern.ch>',
    'ZmumuB_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZmumuC_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZmumuHPTPTV_v221':      'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZmumuHPT_v221':         'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZmumuL_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZnunuB_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZnunuC_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZnunuHPTPTV_v221':      'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZnunuHPT_v221':         'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZnunuL_v221':           'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZtautauB_v221':         'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZtautauC_v221':         'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZtautauHPTPTV_v221':    'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZtautauHPT_v221':       'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZtautauL_v221':         'Paul Thompson <paul.daniel.thompson@cern.ch>',
    'ZZ':                    ''
}


parser = argparse.ArgumentParser(
    description='A script to validate the XSections_13TeV.txt')
parser.add_argument('-f', '--file',
                    help='Custom path of Xsectionfile. '
                    '(Per default the one in $TestArea is used)')
parser.add_argument('-c', '--cxaod', dest='path',
                    help='Find all CxAODs in PATH and check that they '
                    'are in the yield file.')
usage_examples = '''
example usage:
  1) check the default yield file in the CxAODOperations/data that belongs to $WorkDir_DIR
    {0}
  2) check a specified yield file (relative or absolute path)
    {0} -f /path/to/file
  3) check if all channels for which we have CxAODs in a specified folder exist in the cross section file
    {0} -cxaod /eos/atlas/atlascerngroupdisk/phys-exotics/CxAOD/CxAOD_30/HIGG5D1_13TeV/CxAOD_00-30-02_a'''.format(__file__)
try:
    args = parser.parse_args()
except SystemExit as e:
    print usage_examples
    sys.exit(1)

if args.path and not os.path.isdir(args.path):
    print('Error: {} is not a valid directory'.format(args.path))
    sys.exit(1)

if not args.file:
    try:
        testArea = os.environ['TestArea']
    except KeyError:
        print('Error: Environment variable TestArea is not set, cannot search '
              'for Xsectionfile')
        sys.exit(1)
    if not (testArea.endswith('source') or testArea.endswith('source/')):
        testArea = os.path.join(testArea, '..', 'source')
        if not os.path.isdir(testArea):
            print("Error: Cannot find the source area from your environment "
                  "variable TestArea '{}'".format(testArea))
            sys.exit(1)
    xsfile = os.path.join(testArea,
                          'CxAODOperations', 'data', 'XSections_13TeV.txt')
else:
    xsfile = args.file

if not os.path.isfile(xsfile):
    print('Error: {} is not a valid file.'.format(xsfile))
    sys.exit(1)

print('I am checking the file {}'.format(xsfile))

# copy lines into memory
with open(xsfile) as f:
    lines = f.readlines()

# compile a list of duplicated channel entries
duplicates = []
all_chan = []
for line in lines:
    if not line or line.isspace():
        continue
    if line.startswith('#'):
        continue
    try:
        contents = line.split()
    except:
        # ignore unsplittable lines
        continue
    try:
        chan = int(contents[0])
    except:
        # ignore lines where the first column isn't an integer
        continue
    # check if it is existing, keep track of duplicates
    if chan in all_chan:
        if chan not in duplicates:
            duplicates.append(chan)
    all_chan.append(chan)

if duplicates:
    print('Found duplicate entries!')
    for chan in duplicates:
        print('\nChannel {}'.format(chan))
        print('--------{}'.format('-'*len(str(chan))))
        linenumber = 0
        for line in lines:
            linenumber += 1
            line = line.rstrip('\n')
            if line.startswith(str(chan)):
                print('{0}:{1}: {2}'
                      .format(os.path.basename(xsfile), linenumber, line))
else:
    print('No duplicate entries were found.')

if args.path:
    # compile list of cxaods in args.path
    print('\nNow checking if all CxAODs in {} are reflected in the cross section'
          ' file.'.format(args.path))
    print('Compiling a list of all CxAODs, this might take a few seconds...')
    all_cxaods = []
    for root, dirnames, _ in os.walk(args.path,followlinks=True):
        for dirname in dirnames:
            if '.root' in dirname:
                all_cxaods.append(os.path.join(root.split('/')[-1], dirname))

    all_channels = {}  # keys: sample name (from directory)
                       # values: channel numbers for that sample name
    # categorize the cxaods usign their sample name (mainly for printing)
    for cxaod in all_cxaods:
        # split into sample (whatever is before the slash) and cxaod
        try:
            sample = cxaod.split('/')[0]
            cxaod = cxaod.split('/')[1]
        except:
            print('Warning could not split {} into sample and cxaod-dir'
                  .format(cxaod))
            continue

        # skip data
        if sample.startswith('data'): continue
        # find the chan, which is the first integer 
        for part in cxaod.split('.'):
            chan = None
            try:
                chan = int(part)
            except:
                continue
            if not chan:
                print('Warning: Could not determine channel for file {}'
                      .format(cxaod))
                continue
            if sample in all_channels:
                all_channels[sample].append(chan)
            else:
                all_channels[sample] = [chan]

    # check if the channel numbers are in the cross section file
    # keep track of missing channels
    missing_channels = {}
    for sample, channels in all_channels.iteritems():
        for chan in channels:
            if not chan in all_chan:
                if sample in missing_channels:
                    missing_channels[sample].append(chan)
                else:
                    missing_channels[sample] = [chan]

    if missing_channels:
        print('There are channels missing in the cross section file!')
        print('The CxAODReader will crash if you are running the reader over one'
              ' of the samples mentioned below')
        sorted_missing = sorted(missing_channels.keys(), key=lambda x:x.lower())
        for sample in sorted_missing:
            channels = missing_channels[sample]
            print('  {} {} missing for sample {}'
                  .format(len(channels),
                          'channel is' if len(channels) == 1 else 'channels are',
                          sample))
            if not sample in responsibles or not responsibles[sample]:
                print('    I do not know who is responsible for {}.'
                      .format(sample))
                print('    If you know, please update the script and make a'
                      ' merge request on'
                      ' https://gitlab.cern.ch/CxAODFramework/CxAODOperations_VHbb')
                print('    The missing channel numbers are')
            else:
                print('    Please report to {} that the channels for'
                      ' {} are incomplete.'
                      .format(responsibles[sample], sample))
                print('    Mention the missing channel numbers')
            long_string = ' '.join(map(str, channels)).rstrip(' ')
            wrapped_string = textwrap.wrap(long_string)
            for ws in wrapped_string:
                print('      {}'.format(ws))
    else:
        print('All channel numbers that I found in {} are already in the file {}'
              .format(args.path, xsfile))
