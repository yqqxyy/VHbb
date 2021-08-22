#!/usr/bin/env python
#
#  Function for taking an input file list of datasets
#  and calculating the number of events from AMI
#
import os, glob, sys
from time import strftime
import subprocess
from shutil import copyfile

from ConfigParser import ConfigParser
from ConfigParser import Error as ConfigError

from helper_production import *

import argparse

class AMI:
    """
    Class to get AMI cross sections.  
    Run-II version
    """
    
    def __init__(self, **kwargs):
        "Setup options"

        # write cross section or number of events        
        self.mode = 'crossSec' # 'Nevts'
        self.outFile='dslist_crossSec.txt'

        self.configFiles    = []
        
        # Overwrite attributes explicitly set
        for k, v in kwargs.items():    setattr(self, k, v)       
        
        print  self.configFiles 

        pass
        
    def doItForOneInputDataset(self,inDS,verbose,debug):
        if True:
            if True:
                # parse the derivation dataset
                (sampleType,sampleRun,sampleText,sampleStage,sampleDerivation,sampleAMITag,sampleTextPruned,sampleAMITagPruned,sampleDerivationPruned)=parse_derivation_dataset(inDS,verbose,debug)
                isGen=False
                if 'EVNT' in inDS:
                    isGen=True
                    pass
                if debug:
                    print "isGen",isGen
                isData=False
                if 'data15_13TeV' in inDS or 'data16_13TeV' in inDS or 'data17_13TeV' in inDS :
                    isData=True
                    pass
                if debug:
                    print "isData",isData
                # choose evgen/daod strings
                crossSec_str="crossSection_mean"
                filtEff_str="GenFiltEff_mean"
                totalEvents_str="totalEvents"
                if debug:
                    print "crossSec_str",crossSec_str
                    print "filtEff_str",filtEff_str
                    print "totalEvents_str",totalEvents_str
                DAOD=False
                if not isData and "DAOD" in inDS:
                    DAOD=True
                    crossSec_str="crossSection"    
                    filtEff_str="genFiltEff"
                    pass
                if debug:
                    print "DAOD",DAOD
                    print "crossSec_str",crossSec_str
                    print "filtEff_str",filtEff_str
                # form ami command
                pycmd='ami show dataset info '
                pycmd += ' %s' % inDS
                if verbose or debug:
                    print 'pycmd:', pycmd
                pycmd = subprocess.Popen(pycmd, shell=True, stdout=subprocess.PIPE)
                #
                successXS=False
                successEff=False 
                successNEvt=False
                if debug:
                    print "Reading line-by-line the pycmd output to extra the AMI info we want"
                for line in pycmd.stdout:
                    if debug:
                        print 'line',line
                    if not isData and crossSec_str in line:  
                        if DAOD:
                            if line.find("approx") < 0 : 
                                successXS=True
                                crossSec=line 
                        else:
                            successXS=True
                            crossSec=line
                    # done if
                    if not isData and filtEff_str in line:  
                        successEff=True
                        filtEff=line
                    # done if
                    if totalEvents_str in line:
                        successNEvt=True
                        nEvt=line                                     
                    # done if
                # end of loop over line of pycmd output
                if debug:
                    print 'success',successXS
                if not isData and not successXS:
                    print 'error for %s'  % inDS
                    crossSec='error'
                    pass
                if not isData and not successEff:
                    print 'Gen Eff error for %s'  % inDS
                    filtEff='error'
                    pass                        
                if not successNEvt:
                    print 'totalEvents error for %s'  % inDS
                    nEvt='0'
                    pass
                #
                if not isData:
                    crossSec=crossSec.replace("crossSection_mean    : ","")
                    crossSec=crossSec.replace("crossSection         : ","")
                    crossSec=crossSec.strip()
                    filtEff=filtEff.replace("GenFiltEff_mean","")
                    filtEff=filtEff.replace("genFiltEff","")
                    filtEff=filtEff.replace(":","")
                    filtEff=filtEff.strip()        
                #        
                nEvt=nEvt.replace("totalEvents","")
                nEvt=nEvt.replace(":","")
                nEvt=nEvt.strip()
                if debug:
                    print 'crossSec', crossSec
                    print 'filtEff', filtEff
                    print 'nEvt', nEvt
                if self.mode=='crossSec' and not isData:
                    # outstring='%10s %10s %-10s %-30s \n'% (sampleRun, crossSec, filtEff, sampleAMITagPruned)
                    outstring='%10s %10s %-40s %-10s \n'% (sampleRun, crossSec, sampleAMITag, filtEff)
                    print outstring
                else:
                    # outstring='%10s %10s    %-50s %-30s \n'% (sampleRun, nEvt, sampleTextPruned, sampleAMITagPruned)
                    outstring='%10s %10s    %-40s %-60s \n'% (sampleRun, nEvt, sampleAMITag, sampleText)
                    print outstring
                    pass
                # done if
            # done if True
        # done if True
        return outstring,nEvt
    # done function

    def query(self):
        "Loop over inputs and print cross sections, efficiencies or number of events"

        # output file
        outFile = open(self.outFile, 'w')

        print 'mode', self.mode

        for filelist in self.configFiles:
            if debug:
                print 'filelist:',filelist
            file = open(filelist)            
            for inDS  in iter(file):
                # look out for commented, blank lines or short filelists
                if '#' in inDS:
                    continue
                if inDS == "\n": 
                    continue
                if len(inDS)<10:
                    continue
                # strip away the end of line
                inDS=inDS.rstrip()
                if inDS[-1]=='/':
                    inDS=inDS[0:-1]
                if debug:
                    print 'inDS:',inDS
                # sometimes pyami has a glitch and it will return 0 events by mistake
                # if 0 retry for this dataset, but for not more than three times in a row
                nEvt="0"
                for counter in xrange(3):
                    if nEvt!="0":
                        continue
                    if counter>0:
                        print "WARNING! At trial number",counter-1,"the number of events in AMI was zero, so retrying AMI for inDS:"
                        print "inDS",inDS
                    outstring,nEvt=self.doItForOneInputDataset(inDS,verbose,debug)
                # done for loop
                outFile.write(outstring)
                pass
            pass
        outFile.close()
    # done function query

if __name__ == '__main__':

    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter,
                                     description="Get AMI data from list of datasets")     
    parser.add_argument('input', help='input file')     
    parser.add_argument('-of', '--outputFolder', help='output folder', default='./')     
    parser.add_argument('-o', '--output', help='output file', default='dslist_NevtDxAOD.txt')     
    parser.add_argument('-v', '--verbose', action='store_true', help='verbose mode')     
    parser.add_argument('-d', '--debug', action='store_true',help='debugging mode')     

    args = parser.parse_args()     
    print parser.parse_args()

    # inputs
    myfile=args.input
    folder_out=args.outputFolder
    verbose=args.verbose
    debug=args.debug
    outname=args.output

    # run
    filelist=[myfile]
    if debug:
        print 'filelist',filelist
    #  p=AMI(configFiles=[ datafile ]) # for testing one file at a time
    p=AMI(configFiles=filelist)
    p.mode='Nevts' #default is to print cross section, else print number of events
    p.outFile=folder_out+'/'+outname
    p.query()
# done main


