#!/usr/bin/env python

"""
This script counts the number of resulting files outputted from the reader when submitted on a batch system.
Requirement: submitReader.sh should be ran with flag WRITELOGOUTPUT="true"

Current:
    1. Script knows if jobs were ran with JOBSIZELIMIT or NFILESPERJOB
    2. In both case, reads the log output and determines how many jobs should be present in Reader*/fetch/hist-* for each samples.
    3. Prints out a list of samples that are not complete, so the user can simply copy paste them in the submitReader.sh and run only on the failed samples. 
    4. Checks for failed jobs in /status/failed-*
    5. Checks for aborted jobs by comparing the submitted jobs in /submit/segments with the /fetch/done-* files ignoring the failed jobs 
"""


# Script to count the number of batch files in the Reader Output
# Author: Marko Stamenkovic (marko.stamenkovic@cern.ch), Brian Moser (brian.moser@cern.ch)
# Input: 
#   log.out   (Need to turn on WRITELOGOUTPUT="true" in submitReader.sh)

# Import section
import os,glob,sys, re
import argparse

# Parser
parser = argparse.ArgumentParser()
parser.add_argument('--dir', help = 'path/to/reader/output/dir')
args = parser.parse_args()

# Regular expressions
reg_jobsizelimit = 'int jobSizeLimitMB.*= (.+) # 4000'
reg_nfilesperjob = 'int nFilesPerJob.*= (.+) # -1'
reg_fail = '/status/fail-(.+)'
reg_done = '/status/done-(.+)'

# Class
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'



# Methods
# Count number of files in reader output
# Reader/fetch/hist-sample-number.root
def count(loc,sample):
    ls = glob.glob("%s/*%s-*"%(loc,sample))
    return len(ls)

# Find config file
def find_config(path):
    f = glob.glob(path + 'framework-read-automatic*')
    f.sort()
    return f
def find_log(path):
    log = glob.glob(path + 'log*.out')
    log.sort()
    return log

def find_reader_directory(path):
    d = glob.glob(path + 'Reader*')
    d.sort()
    return d 

# Regexp method to find jobSizeLimit and nFilesPerJob in config file
def regexp(reg,config):
    match = re.findall(reg,config)
    if len(match) == 1:
        ret = int(match[0])
        print bcolors.HEADER + "Succesfully retrieved! Value = %d"%ret + bcolors.ENDC
        return ret
    elif len(match) == 0:
        print bcolors.FAIL + "Error: impossible to fetch jobSizeLimit"
        print "Make sure that '%s' is in the file"%(reg) + bcolors.ENDC
        sys.exit(1)
    else:
        print bcolors.FAIL + "Error (ambiguity): retrieved multiple matches"
        print match
        print "Make sure that '%s' is UNIQUE in"%(reg) + bcolors.ENDC
        print config_file
        sys.exit(1)


# Find submission scheme JOBSIZELIMITMB or NRFILESPERJOB
def find_submission_scheme(config_file):
    with open(config_file, 'r') as f:
        config = f.read()
        f.close()

    # Find jobSizeLimit 
    print bcolors.HEADER + "Retrieving jobSizeLimit" + bcolors.ENDC
    jobsizelimit = regexp(reg_jobsizelimit,config)

    # Find nFilesPerJob
    print bcolors.HEADER + "Retrieving nFilesPerJob" + bcolors.ENDC
    nfilesperjob = regexp(reg_nfilesperjob,config)

    if nfilesperjob == -1:
        SizeLimit = True
        reg = 'Sample (.+): nFiles = (.+), nFilesPerJob = (.+)'
    elif jobsizelimit == -1:
        SizeLimit = False
        reg = 'Sample name (.+) with nfiles : (.+)'
    return SizeLimit, reg, nfilesperjob
    
# Compute dictionary with expected samples
def compute_samples_dict(log_path, isSizeLimit, reg, nfilesperjob):
    ret = {}
    with open(log_path, 'r') as f:
        log = f.read()
        f.close()
    matches = re.findall(reg, log)
    if isSizeLimit:
        for match in matches:
            sample = match[0]
            nfiles = int(match[1])
            nFilesperjob = int(match[2])
            expected_files = nfiles // nFilesperjob
            if nfiles % nFilesperjob != 0:
                expected_files += 1
            ret[sample] = expected_files
    else :
        for match in matches:
            sample = match[0]
            nfiles = int(match[1])
            expected_files = nfiles // nfilesperjob
            if nfiles % nfilesperjob != 0:
                expected_files += 1
            ret[sample] = expected_files         
    return ret

def run_matching(config, log, directory):
    IsSizeLimit, reg_sample, nFilesPerJob = find_submission_scheme(config)
    samples_dic = compute_samples_dict(log, IsSizeLimit, reg_sample, nFilesPerJob)

    cmd = ""

    for sample, expected in samples_dic.iteritems():
        fetched = count(directory, sample)
        if fetched == 0:
            print bcolors.WARNING + "WARNING: no files found for %s"%sample + bcolors.ENDC
            #cmd += sample + ' '
        if fetched != expected:
            print "Sample " + bcolors.FAIL + "%s"%(sample)  + ": fetched %d expected %d not matching!"%(fetched, expected) + bcolors.ENDC
            cmd += sample + ' '

        else:
            print "Sample " + bcolors.OKGREEN  + "%s"%sample + ": numbers okay!" + bcolors.ENDC
    print bcolors.BOLD + bcolors.HEADER + "To be copied in the sumbitReader.sh:" + bcolors.ENDC
    print 'SAMPLES=' + bcolors.OKBLUE + '"%s"'%cmd + bcolors.ENDC


def check_for_failed_jobs(directory):
    statii = glob.glob(directory+"*")
    match = [re.findall(reg_fail,s) for s in statii]
    match = [i for x in match if x != [] for i in x]
    return match

def check_for_aborted_jobs(dir_status, dir_submit):
    statii = glob.glob(dir_status+"*")
    match = [re.findall(reg_done, s) for s in statii]
    match = [i for x in match if x != [] for i in x]
    fail_ref = [re.findall(reg_fail,s) for s in statii]
    fail_ref = [i for x in fail_ref if x != [] for i in x]
    with open(dir_submit+"segments") as f:
        lines = f.readlines()
        jobs = [l.split(' ') for l in lines]
        job_ids = [i[0] for i in jobs]
        job_names = [i[1].replace('\n','') for i in jobs]
    aborted = []
    for i in xrange(0,len(job_ids)):
        if job_ids[i] not in match:
            if job_ids[i] not in fail_ref:
                aborted.append(job_names[i])
    if len(aborted)>0:
        print bcolors.WARNING + "WARNING: Aborted jobs found" + bcolors.ENDC
        s_samples = ""
        for s in aborted:
            s_samples += s + " " 
        print "The follwoing jobs have been aborted: "+s_samples
    else:
        print bcolors.OKGREEN + "No aborted jobs found" + bcolors.ENDC

def match_to_samples(nrs, submit):
    n_failed = len(nrs)
    if n_failed > 0:
        print bcolors.WARNING + "WARNING: %i failed jobs found"%n_failed + bcolors.ENDC
        with open(submit+"segments") as f:
            names = dict(re.findall(r'(\S+)\s+(.+)', f.read()))
        s_samples = "Jobs that failed: " + bcolors.FAIL
        for nr in nrs:
            s_samples += names[nr]+" "
        print s_samples + bcolors.ENDC
    else:
        print bcolors.OKGREEN + "No failed jobs found" + bcolors.ENDC 


# Main
if __name__ == '__main__':

    path = args.dir + '/' 

    config = find_config(path)
    log = find_log(path)
    directory = find_reader_directory(path)
    directory_status = [ x + '/status/' for x in directory]
    directory_submit = [ x + '/submit/' for x in directory]
    directory_hist = [ x + '/fetch/' for x in directory]
    directory_tree = [ x + '/fetch/data-MVATree/' for x in directory]
    if len(config) == len(log) and len(config) == len(directory):
        print "Running matching for histograms"
        for i in xrange(len(config)):
            print "Running:"
            print "Config:" + config[i]
            print "Log:" + log[i]
            print "Directory:" + directory[i]
            print "First checking for failed jobs"
            jns_failed = check_for_failed_jobs(directory_status[i])            
            match_to_samples(jns_failed, directory_submit[i])
            print "Checking if jobs have been killed by the scheduler"
            check_for_aborted_jobs(directory_status[i], directory_submit[i])
            print "Now do the histogram matching"
            run_matching(config[i],log[i],directory_hist[i])
            if os.path.isdir(directory_hist[i]):
                print "Running matching for trees"
                run_matching(config[i],log[i], directory_tree[i])
