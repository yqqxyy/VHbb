# Script to hadd large root files containing histograms with systematics outputted by the Reader
# author: Marko Stamenkovic (marko.stamenkovic@cern.ch)

# Import section

import os, glob, re, sys
import argparse
import subprocess
from functools import partial
from multiprocessing.pool import Pool

# Argument parser
parser = argparse.ArgumentParser()
parser.add_argument('--dir', help = 'path/to/output/dir')
parser.add_argument('--out', default = 'inputsFile.root')
parser.add_argument('--multiprocess', action = 'store_true')
args = parser.parse_args()

# Gobal variable
fetch = 'fetch/*.root'
re_sample = 'hist-(.+)-'
max_files = 10
if args.multiprocess:
    max_files = 100

# Method definitions
def get_sample_list(files):
    samples = [re.findall(re_sample,os.path.basename(f))[0] for f in files]
    return list(set(samples))

def count_files(sample, path):
    return len(glob.glob(path + 'hist-%s-*.root'%sample))

def get_cmd(sample, path):
    output = '%s/hist-%s.root'%(path,sample)
    inputs = '%s/fetch/hist-%s-*'%(path,sample)
    if count_files(sample,path + 'fetch/') < max_files:
        cmd = 'hadd -f %s %s'%(output,inputs)
    else:
        cmd = 'hadd -f -j -n 0 %s %s'%(output,inputs)
    return cmd

def hadd_one_sample(sample,path):
    cmd = get_cmd(sample,path)
    output = subprocess.check_output(cmd, shell = True)
    if 'Warning' in output or 'Error' in output:
       print('ERROR: please check' + 'hadd_logs/' + 'log-%s.txt'%sample)
    with open(path + 'hadd_logs/' + 'log-%s.txt'%sample, 'w') as f:
        f.write(output)

# Main section

if __name__ == '__main__':

    readers = glob.glob(args.dir + '/' + 'Reader*')
    readers = [os.path.basename(r) for r in readers]

    for reader in readers:
        print("Running on %s"%reader)
        path = args.dir + '/' + reader + '/'
        files = glob.glob(path + fetch)

        samples = get_sample_list(files)
        if not os.path.exists(path + 'hadd_logs'):
            os.makedirs(path+ 'hadd_logs')
        # Hadd all the samples per sample

        print("Found %d unique samples to hadd."%len(samples))
        if args.multiprocess:
            pool = Pool()
            func = partial(hadd_one_sample,path = path)
            for i,_ in enumerate(pool.imap_unordered(func,samples)):
                 sys.stdout.write('\r Done %d out of %d (%.2f%%).'%(i, len(samples), float(i)*100/float(len(samples))))
                 sys.stdout.flush()

            pool.close()
            pool.join()
        else:
            for s in samples:
                print("%s : %d"%(s,count_files(s,path + 'fetch/')))
                cmd = get_cmd(s,path)
                output = subprocess.check_output(cmd, shell = True)
                if 'Warning' in output or 'Error' in output:
                    print('ERROR: please check' + 'hadd_logs/' + 'log-%s.txt'%s)
                with open(path + 'hadd_logs/' + 'log-%s.txt'%s, 'w') as f:
                    f.write(output)
        # Hadd final sample

        final_cmd = 'hadd -f %s/%s %s/hist-*.root'%(path,args.out,path)

        print("Hadding everything in %s"%args.out)
        final_output = subprocess.check_output(final_cmd, shell = True)
        with open(path + 'hadd_logs/' + 'log-%s.txt'%args.out, 'w') as f:
            f.write(final_output)




