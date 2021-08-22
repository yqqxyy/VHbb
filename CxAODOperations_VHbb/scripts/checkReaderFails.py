#!/bin/python
import os,sys
import ROOT
# The goal of this script is to check how many jobs failed or histograms are not present

total = len(sys.argv)
# number of arguments plus 1
if total!=2:
    print "You need one argument. I will ABORT!"
    print "Ex: ",sys.argv[0]," Reader_0L_31-10_a_MVA_D1"
    print "Ex: ",sys.argv[0]," MyReaderOutput"
    sys.exit(1)

dir_name = os.getcwd()+'/'+sys.argv[1]

# Get the names and the indices from the segment file of the job that shoudl have succeed
f = open(dir_name+'/submit/segments')
doublets=f.read().split()
index,names=doublets[::2],doublets[1::2]

# Get the file's names from fetch and status dir
f_status,f_fetch = os.listdir(dir_name+'/status/'),os.listdir(dir_name+'/fetch/')

# Final array where the fail jobs will be inserted
FailJobs=[]
nFail, nAbsent, nUnReadable = 0,0,0

# Looping over the submitted jobs
for i in range(len(index)):
    HasFailedStatus = False
    for f in f_status:
        # if a fail status is found set the job the array and continue to the next one
        if 'fail-'+ str(i) in f:
            FailJobs.append([index[i],names[i]])
            HasFailedStatus = True
            nFail+=1
            continue
    if HasFailedStatus:
        continue
    
    # Look if the root file has been produced and if it is readable
    isFileAbsent,isFileReadable = True , False
    for f in f_fetch:
        if names[i] in f:
            isFileAbsent = False
            if ROOT.TFile.Open(dir_name+'/fetch/'+f):
                isFileReadable = True
            continue
    
    if isFileAbsent :
        FailJobs.append([index[i],names[i]])
        nAbsent+=1
        continue
    if not isFileReadable :
        FailJobs.append([index[i],names[i]])
        nUnReadable+=1

# Summarize the situation for the user and give him/her some instruction to resubmit the files
print '{}/{} jobs have failed, {} with fail-* file, {} w/o any root file, {} because of unreadable root file'.format(len(FailJobs),len(index),nFail, nAbsent, nUnReadable)
print 'You can resubmit them with:'
for fail in FailJobs:
    print 'bsub -q 8nh -L /bin/bash '+dir_name+'/submit/run '+fail[1],
    if fail is not FailJobs[len(FailJobs)-1]:
        print ' && ',
