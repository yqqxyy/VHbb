Quick Guide to the scripts in the package
-----------------------------------------
-----------------------------------------

For more details on some of these scripts see the Wiki of this package
https://gitlab.cern.ch/CxAODFramework/CxAODOperations_VHbb/wikis/home

The scripts not yet documented have not been checked recently and may be removed if no longer used.

Create Input Sample Lists
-------------------------
```
runAllCreateSampleList.py
createSampleList.py
```

Run Yields Files
----------------
```
count_Nentry_SumOfWeight.py (original)
CreateYieldFiles.py         (updated faster version)    

post-processing corrections:
correct_SumOfWeight.py
```

Get Info from AMI
-----------------
```
runAllGetAMIInfo.py
getAMIInfo.py
```

Cross check Yields and AMI
-------------------------
```
runAllCheckYields.sh
checkYields.py

similarly check for subsequent extensions:
runAllCheckForExtensions.sh
checkForExtensions.py
```

Check PMG Cross Sections
------------------------
```
checkPMGXSecs.py
```

Copy to EOS/disk etc.
---------------------
```
copy_CxAODs_to_eos.py
copy to grid space:
runMakeLinksToGridSpace.sh
makeLinksToGridSpace.py
```

Replicate to Grid
-----------------
```
replicateToGrid.py
```

Run Test Maker jobs
-------------------
```
runTestFiles.sh
```

Submit Reader/Maker
-------------------
```
submitMaker.sh
submitReader.sh
```

Merge large histograms with systematics
-------------------

### Details of the implementation
The script requires as an input the output repository produced by `submitReader.sh`. The script does the following:
1. Finds MC production repository written out by the `submitReader.sh`
2. For each repository, finds the unique list of samples (automatically, no hardcoded samples in the script) inside the `fetch` repository.
3. Produce for each sample a root file (`hadd`) and writes it the repository. If the number of partial samples exceeds 10, the `hadd` is ran in parallel mode. The script writes a log output per samples in a repository named `hadd_logs`. 
4. Once all the root files per samples are produced, the script merges them into a single file named by default `inputsFile.root` (this option can be specified at shell level. 

The final merger of the different MC productions should be done manually. 

### Usage
```
cd source/CxAODOperations_VHbb
python scripts/haddReaderHists.py --dir /path/to/reader/output/ --out inputsFile.root
```
A `--multiprocess` options exists but it turns out to not work so well for large number of samples. 

utils directory
---------------

Not yet documented
-------------------
```
checkReaderFails.py
checkSubmissionWorkspace.py
checkXsectionfile.py
cleanPrepareFolder.sh
compareSampleList.py
countReaderBatchFiles.py
createOutputLisFromAlreadyDownloaded.sh
createSampleListAnalysis.py
createSampleListPRW.py
downloadPRW.py
form_HSG5_out.py
getMaker.sh
helper_production.py
killRucioPID.sh
manipulateReader.sh
operatePandaJobs.sh
prepareCxAODProduction.sh
rucio_get_jobs.py
runAllCreateSampleListPRW.py
runAllPrepareFolder.sh
runRepeatedlyRucioGetJobs.sh
setupDownload.sh
setupGrid.sh
setupLocal.sh
splitSampleList.py
studySampleList.py
testLocallyAsInCIPipeline.sh
updateOutputSampleList.sh
updateSampleInfo.sh
```
