# Script to correct the sum of weights in cases of errors occuring directly from the production where a few number of events have an insanely large MC weight
# See https://indico.cern.ch/event/799812/contributions/3323526/attachments/1799161/2934022/intro_hbb_19-02-20.pdf
# Files concerned
#
# ggZllHbb: MC16d 
# ggZvvHbb: MC16e
# ggZllHcc: MC16e
# ggZvvHcc: MC16e
#
# Usage:
# This script needs to be copied on eos where the samples are stored (along with count_Nentry_SumOfWeight.py)
# author: Marko Stamenkovic

# Import section
import os, re, glob, sys
from collections import OrderedDict

# Global variables
script_path = os.path.dirname(os.path.realpath(__file__))

dsids = { 'ggZllHbb_PwPy8' : ['345057'],
          'ggZvvHbb_PwPy8' : ['345058'],
          'ggZllHcc_PwPy8' : ['345113'],
          'ggZvvHcc_PwPy8' : ['345114'],
        }

eos_path = '/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-15'

channels = { '2L' : 'HIGG2D4_13TeV',
            '1L' : 'HIGG5D2_13TeV',
            '0L' : 'HIGG5D1_13TeV',
        }

MCprods = { 'MC16a' : 'CxAOD_32-15_a',
            'MC16d' : 'CxAOD_32-15_d',
            'MC16e' : 'CxAOD_32-15_e',
        }

matchings = OrderedDict()

#matchings['ggZllHbb_PwPy8-2L-MC16d'] = 'ggZllHbb_PwPy8-2L-MC16a'
#matchings['ggZvvHbb_PwPy8-2L-MC16e']= 'ggZvvHbb_PwPy8-2L-MC16a'
#matchings['ggZllHcc_PwPy8-2L-MC16e']= 'ggZllHcc_PwPy8-2L-MC16a'
#matchings['ggZvvHcc_PwPy8-2L-MC16e']= 'ggZvvHcc_PwPy8-2L-MC16a'

#matchings['ggZllHbb_PwPy8-1L-MC16d']= 'ggZllHbb_PwPy8-1L-MC16a'
#matchings['ggZvvHbb_PwPy8-1L-MC16e']= 'ggZvvHbb_PwPy8-1L-MC16a'
#matchings['ggZllHcc_PwPy8-1L-MC16e']= 'ggZllHcc_PwPy8-1L-MC16a'
#matchings['ggZvvHcc_PwPy8-1L-MC16e']= 'ggZvvHcc_PwPy8-1L-MC16a'

#matchings['ggZllHbb_PwPy8-0L-MC16d']= 'ggZllHbb_PwPy8-0L-MC16a'
#matchings['ggZvvHbb_PwPy8-0L-MC16e']= 'ggZvvHbb_PwPy8-0L-MC16a'
#matchings['ggZllHcc_PwPy8-0L-MC16e']= 'ggZllHcc_PwPy8-0L-MC16a'
#matchings['ggZvvHcc_PwPy8-0L-MC16e']= 'ggZvvHcc_PwPy8-0L-MC16a'


yields = 'yields.13TeV_sorted.txt'

# Method definition

def compute_regexp_pattern(dsid):
    pattern = ' %s (.+) (.+) (.+)'%(dsid)
    return pattern

def get_path(channel,MCprod):
    return "%s/%s/"%(channels[channel], MCprods[MCprod])

def parse_yields(yields_file, dsid):
    pattern = compute_regexp_pattern(dsid)
    matches = re.findall(pattern, yields_file)
    if len(matches) != 1 :
        print "Error zero or multiple matches found in yield file for DSID:%s"%dsid
        sys.exit(1)
    else: 
        DxAOD_yields = float(matches[0][0])
        CxAOD_yields = float(matches[0][1])
        DxAOD_SoW = float(matches[0][2])
        return DxAOD_yields, CxAOD_yields, DxAOD_SoW


def find_files_path(inp):
    dev = inp.split('-')
    path_dev = eos_path + '/' + get_path(dev[1],dev[2]) + yields
    string_ref = matchings[inp]
    metadata_ref = string_ref.split('-')
    sample_ref = metadata_ref[0]
    channel_ref = metadata_ref[1]
    MCprod_ref = metadata_ref[2]
    dsid_ref = dsids[sample_ref]
    path_ref = eos_path + '/' + get_path(channel_ref,MCprod_ref) + yields
    return dsid_ref, path_dev ,path_ref

def run_correction(label):
    dsid_ls, path_dev, path_ref = find_files_path(label)
    print "Running on DSID:", dsid_ls
    print "Dev: %s"%path_dev
    print "Ref: %s"%path_ref
    with open(path_dev, 'r') as f:
        dev = f.read()
    dev_to_change = str(dev)

    while '  ' in dev: dev = dev.replace('  ',' ')

    with open(path_ref, 'r') as f:
        ref = f.read()
    while '  ' in ref: ref = ref.replace('  ',' ')

    for dsid in dsid_ls:
        dev_DxAOD, dev_CxAOD, dev_SoW = parse_yields(dev, dsid)
        ref_DxAOD, ref_CxAOD, ref_SoW = parse_yields(ref, dsid)

        new_SoW = ref_SoW * dev_DxAOD / ref_DxAOD
        old_line = '%10s %10.0f %10.0f %15f'%(dsid, dev_DxAOD, dev_CxAOD, dev_SoW)
        ref_line = '%10s %10.0f %10.0f %15f'%(dsid, ref_DxAOD, ref_CxAOD, ref_SoW)
        new_line = '%10s %10.0f %10.0f %15f'%(dsid, dev_DxAOD, dev_CxAOD, new_SoW)
        dev_to_change = dev_to_change.replace(old_line, new_line)
        print "Replacement in %s"%label
        print "Old line: %s"%old_line
        print "Ref line: %s"%ref_line
        print "New line: %s"%new_line
#    print parse_yields(yields_sorted, dsid)
    print "Writing file under: %s"%path_dev
    with open(path_dev, 'w') as f:
        f.write(dev_to_change)

# Main section

if __name__ == '__main__':
    # Example to correct ggZllHbb_PwPy8 weights in 2-lepton MC16d with ggZllHbb_PwPy8 weights in 2-lepton MC16a
    matchings['ggZllHbb_PwPy8-2L-MC16d'] = 'ggZllHbb_PwPy8-2L-MC16a'  # Key = file to be corrected, value = file to take as reference 
#    matchings['ggZvvHbb_PwPy8-2L-MC16e'] = 'ggZvvHbb_PwPy8-2L-MC16a'
#    matchings['ggZllHcc_PwPy8-2L-MC16e'] = 'ggZllHcc_PwPy8-2L-MC16a'
#    matchings['ggZvvHcc_PwPy8-2L-MC16e'] = 'ggZvvHcc_PwPy8-2L-MC16a'

#    matchings['ggZllHbb_PwPy8-1L-MC16d'] = 'ggZllHbb_PwPy8-1L-MC16a'
#    matchings['ggZvvHbb_PwPy8-1L-MC16e'] = 'ggZvvHbb_PwPy8-1L-MC16a'
#    matchings['ggZllHcc_PwPy8-1L-MC16e'] = 'ggZllHcc_PwPy8-1L-MC16a'
#    matchings['ggZvvHcc_PwPy8-1L-MC16e'] = 'ggZvvHcc_PwPy8-1L-MC16a'

#    matchings['ggZllHbb_PwPy8-0L-MC16d'] = 'ggZllHbb_PwPy8-0L-MC16a'
#    matchings['ggZvvHbb_PwPy8-0L-MC16e'] = 'ggZvvHbb_PwPy8-0L-MC16a'
#    matchings['ggZllHcc_PwPy8-0L-MC16e'] = 'ggZllHcc_PwPy8-0L-MC16a'
#    matchings['ggZvvHcc_PwPy8-0L-MC16e'] = 'ggZvvHcc_PwPy8-0L-MC16a'
    
    affectedRepository = False # Set to true in the repositories that need to be corrected for, please specify which 
    if affectedRepository:
        for label in matchings:
            run_correction(label)

