***This project is part of the [CxAODFramework](https://gitlab.cern.ch/CxAODFramework) the [wiki](https://gitlab.cern.ch/CxAODFramework/CxAODOperations_VHbb/wikis/home) contains detailed documentation.***

# Difference of old and new style

The new style improves the experience of both developer and user of the various packages, by really decoupling:

* Maker and Reader
* core and VHbb
* code and operations

To achieve that, `FrameworkExe` and `FrameworkSub` are not used any more, as per the discussions in GitLab issue of [FrameworkSub](https://gitlab.cern.ch/CxAODFramework/FrameworkSub/issues/33) and [FrameworkExe](https://gitlab.cern.ch/CxAODFramework/FrameworkExe/issues/10). Instead, their elements have been spread around the existing packages, as well as in new packages:

* `CxAODBootstrap_VHbb`
* `CxAODOperations`
* `CxAODOperations_VHbb`

Here is the list of what has been moved to new packages:

* `FrameworkSub/boostrap` -> `CxAODBootstrap_VHbb/bootstrap`. Unlike for `FrameworkSub`, there is no `.root` file to check out, decoupling things nicely.
* `FrameworkSub/data/XSections_13TeV.txt` -> `CxAODOperations/data/XSections_13TeV.txt`. In a core package, as has cross sections for analyses other than those using the `VHbb` CxAOD production, like `bbtautau`. 
* `FrameworkSub/README.md` -> `CxAODOperations_VHbb/README.md` (in fact two README, one for `Maker`, one for `Reader`), as they describe in reality the operations
* Add in `CxAODBootstrap/README.md` the info on how to check out the code (the very file you are reading now)
* The scripts of `FrameworkExe/scripts` -> `CxAODOperations_VHbb/scripts` (but some scripts not used in `R21` have not been copied over, thus cleaning a bit)
* `FrameworkSub/In/VHbb` -> `CxAODOperations_VHbb/data/DxAOD/VHbb`
* `FrameworkSub/data/sample_info.txt` -> `CxAODOperations_VHbb/data/DxAOD/info/sample_info.txt`
* `FramworkSub/Out/CxAOD_r31-10` -> `CxAODOperations_VHbb/data/CxAOD/CxAOD_r31-10`
* `FramworkSub/Out/CxAOD_r31-15` -> `CxAODOperations_VHbb/data/CxAOD/CxAOD_r31-15`
* The files from `FramworkSub/data` -> `CxAODOperations_VHbb/data/CxAOD/info`, e.g. yields files (which are not used in the new format of `submitReader.sh` and two TMVA which seem to be  for b-jet energy Regression usable at Reader level, but maybe I misread it?)

Here is the list of what has been moved to already existing packages:

* `FrameworkExe/util/hsg5framework.cxx` -> `CxAODMaker_VHbb/util/hsg5framework.cxx`
* `FraweorkExe/data/framework-run*.cfg` -> `CxAODMaker_VHbb/data/framework-run*.cfg`
* `FrameworkExe/util/hsg5frameworkCxAODRead.cxx` -> `CxAODReader_VHbb/util/hsg5frameworkCxAODRead.cxx`
* `FrameworkExe/data/framework-read*.cfg` -> `CxAODReader_VHbb/data/framework-read*.cfg`

# What changes for you as user

To checkout and compile, nothing changes, as explained above with `getMaster.sh`. As a user, almost nothing changes. Just the scripts and samples for operations are no longer in `FrameworkExe` and `FrameworkSub`, but in `CxAODOperations_VHbb`.  

# ROOT version on lxplus

Since morning of 16 Jan 2018, you need to add to your `.bashrc` or equivalent.
```
export ALRB_rootVersion=6.14.04-x86_64-slc6-gcc62-opt
```
This is the latest version of ROOT. You can choose other, if you want. The change to this new one was recent, and it posed some problems to some analyses, so I guess they want to give the ability to switch back to the earlier version for them. If you do not set this veriable, and you just run `lsetup root`, it will not set by default the latest root (as it did as until now), but it will tell you you need an extra argument, to specify exactly which ROOT you want.
```
-bash-4.1$ lsetup root
Error: You need to specify a root version.  The current recommendation is
         lsetup "root 6.14.04-x86_64-slc6-gcc62-opt" 
		  or 
         export ALRB_rootVersion=6.14.04-x86_64-slc6-gcc62-opt
         lsetup root
       Please consult your analysis group if you need a specific version for
       your work.  To see what versions are available, type
         showVersions root
```

Many people reacted to this and complained, suggesting if we set the ROOT version, we will forget about it in a few years and realise we had not been using the latest version of ROOT.
Their suggestions are, in order of preference:
* restore `lsetup root` to do, as until yesterday, to set up the latest ROOT
* make available `lsetup root latest` or similar, for the vast majority of user cases that are happy to use the latest version of ROOT.

# Check out and compile in one go Maker and Reader at the same time

Tag `r31-18` is the first tag of the new style. It has the same physics as `r31-17`, the last tag of the old style. Both have been confirmed to reproduce the ICHEP inputs for `0L`. The `CxAODFramework` repositories are re-organised, with no change to the code. The changes are transparent to the user. The checkout is done exactly the same was as before. The script `getMaster` (https://gitlab.cern.ch/CxAODFramework/CxAODBootstrap_VHbb/blob/master/scripts/getMaster.sh) has been made backward compatible. For a branch or for a tag `r31-18` or newer, it will use the new style, with `CxAODBootstrap_VHbb`. For a tag `r31-17` or older, it will use the old style with `FrameworkSub`. Run the script 

```
source /afs/cern.ch/user/v/vhbbframework/public/CxAODBootstrap_VHbb/scripts/getMaster.sh
```

It will crash in a controlled way and tell you what the options are, telling you for the branch what is the latest AnalysisBase and for the tag what is the latest tag. These values you can also find out directly from GitLab here for [AnalysisBase](https://gitlab.cern.ch/CxAODFramework/CxAODBootstrap_VHbb/blob/master/bootstrap/release.txt) and from here for [the latest tag](https://gitlab.cern.ch/CxAODFramework/CxAODBootstrap_VHbb/tags).

To checkout and compile the head of the master branch. For example, as of 12 Jan 2019, the latest are:
```
source /afs/cern.ch/user/v/vhbbframework/public/CxAODBootstrap_VHbb/scripts/getMaster.sh origin/master          CxAODFramework_branch_master_21.2.59_1    1           1
```

To check out and compile a new tag, either in the old format or in a new format, do like
```
source /afs/cern.ch/user/v/vhbbframework/public/CxAODBootstrap_VHbb/scripts/getMaster.sh r32-07                 CxAODFramework_tag_r32-07               1           1
```

# How to check out only Maker, only Reader, or both

Currently by default, both `Maker` and `Reader` are checked out for both `core` and `VHbb`, just as before. But unlike before, now you have the ability to check out only `Maker` or only `Reader`. That makes the compilation faster. And it is also fundamentally nice to not use code you do not actually use. You would have to comment out the packages you do not want in 

https://gitlab.cern.ch/CxAODFramework/CxAODBootstrap_VHbb/blob/master/bootstrap/packages_VHbb_git.txt

For `Maker`, you would need:
* `CxAODOperations`
* `CxAODOperations_VHbb`
* `CxAODMaker`
* `CxAODMaker_VHbb`
* `CxAODTools`
* `CxAODTools_VHbb`

For `Reader`, you would need:
* `CxAODOperations`
* `CxAODOperations_VHbb`
* `CxAODReader`
* `CxAODReader_VHbb`
* `CxAODTools`
* `CxAODTools_VHbb`
* `CorrsAndSysts`
* `KinematicFit`

But how do you do this in practice? The `getMaster.sh` does it all in one go. Well, it has two optionsn that allows you to stop where you want. The last two arguments are too booleans. If the first boolean is `0`, it stops after checking about the `CxAODBootstrap_VHbb`. This allows you to manually add the list of packages, then manually continue by hand, see below. So check out with 

```
source /afs/cern.ch/user/v/vhbbframework/public/CxAODBootstrap_VHbb/scripts/getMaster.sh origin/master          CxAODFramework_branch_master_21.2.58_01    0           1
```
You are left in the folder `source`. Now you can edit the file.

```
emacs -nw CxAODBootstrap_VHbb/bootstrap/packages_VHbb_git.txt
```

By the way, you can also change there the particular branch you want to check out, or for a given branch a particular tag, or a particular commit. So it gives you flexibility when developing to take some branches from master, while others are your own development branches.

Then you continue with the check out of the other packages and then the compilation by hand, see below.

# Bump to a new AnalysisBase version and test if it works fine

Let's assume currently the AnalysisBase is `21.2.58` and we want to update to `21.2.59`, test it all works, and then make a MR. We check out all the packages (firs number is 1), but we do not compile (second number is 0). Running without any argument will tell us that the latest `AB` is `21.2.58`.

```
Usage: -bash origin/master          CxAODFramework_branch_master_21.2.58_1    1           1
```
Since we will change eo `59` we use that instead as the name of the folder. So we run:

```
source /afs/cern.ch/user/v/vhbbframework/public/CxAODBootstrap_VHbb/scripts/getMaster.sh origin/master          CxAODFramework_branch_master_21.2.59_2 1 0
```

This checks out all the packages and leaves us in the `source` folder. To change the Analysis Base version we modify this file.

```
emacs -nw CxAODBootstrap_VHbb/bootstrap/release.txt 
```
and update `21.2.58` with `21.2.59`.

Then we need to set up the AnalysisBase version and compile. That is why we did not do that directly by putting 0 as the second number.

```
cd ..build
```
Now there are a few commands to run by hand, but they are also in a script, to make it easy for us
```
emacs -nw ../source/CxAODBootstrap_VHbb/bootstrap/compile.sh
# assumes we are in the build folder                                                                                        
setupATLAS
lsetup asetup
release=`cat ../source/CxAODBootstrap_VHbb/bootstrap/release.txt`
echo "release=$release"
asetup AnalysisBase,$release,here
cp CMakeLists.txt ../source
cmake ../source
cmake --build .
source x86_64-slc6-gcc62-opt/setup.sh
```
We need to source, as it needs to `setupATLAS`. Now we run.

```
source ../source/CxAODBootstrap_VHbb/bootstrap/compile.sh
```

Now it compiled and sourced the setup, so you are ready to use it, like testing it locally. 

```
source /afs/cern.ch/user/v/vhbbframework/public/CxAODBootstrap_VHbb/scripts/getMaster.sh
```

And you are ready to run the executables for `Maker` or `Reader`. 

Remember that if you do not want to change anything, there is the `getMaster.sh` script that does it all for you in one go.

# Re-compile after further changes

As you develop the code, you need to recompile again, as below.

```
cd build
```

If you added at least a new file you need to do
```
cmake ../source 
```

If you added or modified at least a file do
```
cmake --build .
```

This step is not needed again, but in case it is not working, do it again.
```
source x86_64-slc6-gcc62-opt/setup.sh 
```

To make merge requests after your change, or to check out some Athena package that is needed for your code to run, see the dedicated [README_develop.md](https://gitlab.cern.ch/CxAODFramework/CxAODBootstrap_VHbb/blob/master/README_Develop.md).

# asetup for other times you log in

To be able to restore the asetup settings after a compilation for the next times you log in, make sure you have this file, `${HOME}/.asetup`, and inside the lines

```
[defaults]
autorestore = True
```

Then every time you log in again you simply go to the `build` folder of your `CxAOD` folder.

```
ssh -Y lxplus.cern.ch
cd CxAODFramework_1
cd build
asetup
source x86_64-slc6-gcc62-opt/setup.sh
```

# Testing CxAOD Maker locally

We un in parallel locally for many test cases. Combinations of all channels, periods, processes we want to test. It is the same we have in the pipeline. The workflow is to test locally first and only make MR when after development this test passes for all samples.

You are now in `build`. Do:

```
cd ../run
ls
```
You will see already `configs logs` folders, and a script `clean.sh` that would remove all the `submitDir` folders, the config and the log files after a test, so that you can start fresh with a new test. You can now run a test with

```
source ../source/CxAODOperations_VHbb/scripts/testLocallyAsInCIPipeline.sh 
```

This take as argument a list of samples that will be run in parallel. You can use those already defined.
```
Usage: -bash testLocallyAsInCIPipelineTasks.txt
Usage: -bash testLocallyAsInCIPipelineTasksReduced.txt
Usage: -bash testLocallyAsInCIPipelineTasks2.txt
```
which are located at
```
cd source/CxAODOperations_VHbb/data/DxAOD/info
ls
```

This one `testLocallyAsInCIPipelineTasks.txt` contains all the samples available on `afs` in the `CxAODFramework` bot account. For each channel and period, there is one signal, one data and `ttbar`. The number represents the number of events to run on. You can set to `-1` to run on all.

This one `testLocallyAsInCIPipelineTasksReduced.txt` has just the `1L e` with three processes, in case you want to test quickly something. `1L` is the most generic, having all the objcts. And period `e` ensures you have all set up up to the latest period. 

It looks like this
```
1L_data18        1L e VHbb   1000  VHbb/data18_13TeV.00364292.physics_Main.deriv.DAOD_HIGG5D2.f1002_m2037_p3640
1L_mc16e_WHlvbb  1L e VHbb    200  VHbb/mc16_13TeV.345053.PowhegPythia8EvtGen_NNPDF3_AZNLO_WmH125J_MINLO_lvbb_VpT.deriv.DAOD_HIGG5D2.e5706_e5984_s3126_r10724_r10726_p3641
1L_mc16e_ttbar   1L e VHbb   1000  VHbb/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_HIGG5D2.e6337_e5984_s3126_r10724_r10726_p3639
```

Where are these datasets located? At 
```
pushd  /afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/
ls
popd
```

You can comment out these lines to ignore them, you can add others. 

The logs 
```
cd logs 
ls
less Maker_0L_data15.log
tail -f  Maker_0L_data15.log
cd ..
```
And you see the output. Normally it works. With `ls -lrt` it usually shows in the terminal which one ran and which one failed. Usually they all run. If that is fine, then you can make a MR of your developement. 

The output `submitDir` folders are outside directly, like 
```
ls
Maker_0L_data15  Maker_0L_mc16a_HVT     Maker_0L_mc16d_ZHvvbb  Maker_1L_data16     Maker_1L_mc16a_WpHlvbb  Maker_1L_mc16d_ttbar  Maker_2L_data17        Maker_2L_mc16a_ttbar   clean.sh
Maker_0L_data16  Maker_0L_mc16a_ZHvvbb  Maker_0L_mc16d_ttbar   Maker_1L_data17     Maker_1L_mc16a_ttbar    Maker_2L_data15       Maker_2L_mc16a_HVT     Maker_2L_mc16d_ZHllbb  configs
Maker_0L_data17  Maker_0L_mc16a_ttbar   Maker_1L_data15        Maker_1L_mc16a_HVT  Maker_1L_mc16d_WpHlvbb  Maker_2L_data16       Maker_2L_mc16a_ZHllbb  Maker_2L_mc16d_ttbar   logs
cd  Maker_0L_mc16d_ZHvvbb 
ls 
```


This in turns simply loops over them and calls `submitMaker.sh` for each. You can also download one new `DxAOD` dataset from the grid, and run on it locally directly with `submitMaker.sh`. Run it without any argument and you will get an example, such as. 

```
source ../source/CxAODOperations_VHbb/scripts/submitMaker.sh
source ../source/CxAODOperations_VHbb/scripts/submitMaker.sh 0L a VHbb none none Higgs 0 0 1000 /afs/cern.ch/work/v/vhbbframework/public/data/DxAOD/VHbb/data16_13TeV.00311321.physics_Main.deriv.DAOD_HIGG5D1.r9264_p3083_p3640 none 1
```
If you replace `none` with `long` or with a particular string, it will run the process in the background and put the ouput in a log file. That allows to submit several processes in parallel. 

Read more about running the `CxAODMaker` in the dedicated [README](https://gitlab.cern.ch/CxAODFramework/CxAODOperations_VHbb/blob/master/README_RunMaker.md).

# Reading CxAOD in PyRoot with numpy

If in addition you want to open `CxAOD` files in `PyRoot` using a script that needs `numpy`, that is not set up in the usual `AnalysisBase` setup, so you need to do an extra line
```
lsetup 'lcgenv -p LCG_91 x86_64-slc6-gcc62-opt numpy' 
```

# Running `eos ls`

In case the `eos ls` command does not work for you on `lxplus`, you should do 
```
export EOS_MGM_URL=root://eosatlas.cern.ch 
```

# Tag packages in Git

One script sits in [GitLab](https://gitlab.cern.ch/CxAODFramework/CxAODBootstrap_VHbb/blob/master/scripts/createTag.sh) and in `/afs` here
```
/afs/cern.ch/user/v/vhbbframework/public/CxAODBootstrap_VHbb/scripts/createTag.sh
```
Running without arguments crashes in a controlled way and tells what arguments it expects. It expects two arguments. The tag name and a string for the tag name. Edit the file to add a command line argument and see the previous comments. In the text it is placed usually the current date, the AB release of the tag and some major code updates that were added since the previous tag. Add a new line for your entry, to be used for later reference. Now run the script, which makes the tags. I am not sure which permission level you need to have. But for sure `owners` can do that, and there are several in the framework at the moment, and more can be assigned the role. 

The script loops over the packages and creates a tag for each. As such all packages have the same tag, making it easier to check them out afterwards in the bootstrap script. A tag is only a symbolic link to one commit on a branch. So tags are cheap to create. Feel free to create tags as often as you need. 

A pipeline will be triggered automatically that will fail for each package. You can safely ignore it. Our pipelines work for the case of a first commit, but not when a merge request is actually merged, or when a tag is already done.

Once the tag is ready, update the script that checks out the package. We want it to always up to date so that a generic user can find what is the latest tag to check out, if they want to. 

```
emacs -nw /afs/cern.ch/user/v/vhbbframework/public/CxAODBootstrap_VHbb/scripts/getMaster.sh 
```
For example update at the top where the command line arguments are shown
```
Usage: $0 r31-23                 CxAODFramework_tag_r31-23_1               1           1  
```
with 
```
Usage: $0 r31-23                 CxAODFramework_tag_r31-24_1               1           1  
```

Then you need to add it by hand to the twiki in the list of merge requests. 

https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/Release21Migration#List_of_Merge_requests

For this you need to ssh to the lxplus account of the `CxAODFramework` (you need its username and password for that), go to the `eos` folder, find out the latest file of the list of merge requests `.txt` to edit by `ls -lrt`, then edit it by adding at the top the tag nummer in the format as for the other tags. The merge requests are added to that file automatically, but for the tags you need to do by hand. Then this file will appear in the browser for the twiki.

# Permission error to write from home institue to eos:

Sometimes using the script `copy_CxAODs_to_eos.py` to copy from your home institute to eos returns an error like this:

```
Error 3010: Unable to create parent directory /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-04-EM
```

This is related to the permission to copy from your home institute, it can be solved by sourcing this script:

```
export KRB5CCNAME=${HOME}/.aklog_cache
kinit <username>@CERN.CH
aklog CERN.CH
klist
```

## Adding .root files to the code

The `.root` files are not stored in `GitLab`, as not recommended by ATLAS. They are stored instead in `/afs` of the `CxAODFramework` bot account.
```
/afs/cern.ch/work/v/vhbbframework/public/data
```

These are copied in the our framework's code by the bootstrap script. The exact `/afs` location is defined in the bootstrap script in the [setup.sh](https://gitlab.cern.ch/CxAODFramework/CxAODBootstrap_VHbb/blob/master/bootstrap/setup.sh#L24), which currently is 
```
DATA_PREFIX="/afs/cern.ch/work/v/vhbbframework/public/data/dataForCxAODFramework_181026"
```

# Pile-up reweighting (PRW) files

The `PRW` files are currently at
```
/afs/cern.ch/work/v/vhbbframework/public/data/dataForCxAODFramework_181026/CxAODOperations_VHbb/PRW
```
Note that as of 14 Dec 2018, the symbolic links point to files for the new derivations which only contain the default `410470` (`ttbar`). All other datasets use the the pileup profile of `ttbar`. At some point soon we will update the `PRW` files to contain all the other `DSID` available.

# EventLoop memory check

Since AnalysisBase `21.2.60` an automatic memory leak check has been introduced in `EventLoop`, which is used by both our `Maker` and `Reader`. An example of output from Reader is 
```
Memory increase/change during the job:
  - resident: 0.399318 kB/event (133680 kB total)
  - virtual : 0.366722 kB/event (122768 kB total)

```
[Here](https://gitlab.cern.ch/atlas/athena/merge_requests/20313) is the update in Athena that produces these messages. As you can see from that, we now have a few additional job properties that control how the results of the memory measurements should be handled, like [here](https://gitlab.cern.ch/atlas/athena/commit/3e47f9d150f8f2f826f27d785bc5bcff2cec967c).

