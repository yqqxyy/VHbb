# List of Merge Request

https://twiki.cern.ch/twiki/bin/view/AtlasProtected/Release21Migration#List_of_Merge_requests

The generic webpage is here:

https://twiki.cern.ch/twiki/bin/view/AtlasProtected/Release21Migration

# Create a GitLab issue

If you need to change the code in more than two repositories, you have to create a GitLab issue. If the change is in only one repository, it is also recommended to create a GitLab issue. Choose a title that is short, but with enough detail. Write a full description in the text of the issue. Choose e relevant label. If an appropriate one does not exist, create one. Assign the issue to the person that will do the work (the same person that will ask for a merge request). Then when for each MR, the instructions are below.

# Make a merge request (MR)

Start with `Addressing the issue ` followed the the `url` to the GitLab issue. Since the GitLab issue describes in detail, here you do not have to write in detail. Even just this `Addressing the issue` is enough. Again choose a label and if not present, create one. Follow the templates that are there, but remove all the info that is not needed to the reviewer. Basically choose if it is a bug fix or a feature extension. Make sure you tick the two boxes to squash the several commits into one and to remove the original branch after merge. Assign the merge request to a person to review it. It usually is one of Andreas Hoenle, Stephen Jiggins, Adrian Buzatu. For Reader Hannah Arnold. For Reader MVA, Elisabeth Schoph. 

The branch names must start with `master-` so that the Continuous Integration (CI) pipelines specific for `R21` starts. Follow them after your MR. There are three steps. The first one is the `build` one. This can either fail (red) or succeed (green). The second one checks for compilation warnings. It can either have warnings (orange, but still go on) or no compilation warnings (green). The third and last step is running the `Maker` (in most packages, for `Reader` packages it runs the `Reader` instead), it can be either red or green. If the pipeline has red or orange, comment in the text, see why it failed, make a new commit. Do not just leave sitting like that for a long time. Follow up on your work, to get it merged quickly. 

When you think you are ready, please tag 

To activate the Continuous Integration (CI) pipelines we can not use forks any more, like in R20.7 and CxAOD28, but use use branches. The workflow is explained in detail in the [twiki](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/CxAODContinuousIntegration).

The ideal is that when you start developing a new feature, check out and compile a code again. With the script that does it in one go, it only takes a few minutes. Otherwise, if you use a code that you checked out in the recent past, you risk that the various repositories in the framework have changed in the head of the master branch in the meanwhile. You may change one repository, it may compile and run correctly locally. Yet, when you make a commit to the master for that repository, it triggers a pipeline. This pipeline will use your latest code for that repository, but the latest code for all the other repositories. This may be different from what you have locally! The pipeline may crash! Or it may pass, while your local code actually crashes. You need to make sure that the code runs correctly both locally and on the pipeline.

If you want to use an old code, you should ask yourself why. Is it because you made many commits that would be lost otherwise? Then put the code in Git! You are not ready to put them in master? No worries, put them in your own branch. 

If you really want to use an old checkout, then you must update all the packages to the latest by running in each `git pull origin master`, then re-compile and re-run locally. Only after that you make the merge request to master.

If you do not want your pipeline to run, make the title of the latest commit to start with `[skip ci]`.
```
git log
git commit --amend
```
For the pipeline to run, the branch name must start with `master`. If you used as recommended the default script to check out, the branch name is already `master-` followed by your username. We recommend you update it further to say a few things about your actual commits. 
```
git branch -m "master-abuzatu-update-AnalysisBase-21-2-43"
```
Now you are ready to push to GitLab.
```
git push -u origin master-abuzatu-update-AnalysisBase-21-2-43
```
Now the branch `master-abuzatu-update-AnalysisBase-21-2-43` appears in GitLab page of branches. This also gives you a `url` that you can paste in the browser to start a merge request (MR) of your branch to the master. Note a MR can only be done on the same server. So your branch from GitLab to the master on GitLab.

Choose from the template one of the options, usually either fixing a bug, or adding a new feature. There is text there that explains you what to write. Do not leave that text there! Keep only the text relevant to the reviewer. 

Choose a title that is short, but clear about what the MR is about. Do not keep it vague. If you have work in progress, start your title with `[WIP]`. We recommend that you create a MR as soon as you start developing and then make pushes to the same branch for further milestones in your progress. That will list all the pipelines, all the code changes with respect to the previous version. It is very powerful. And your collaborators will know already that you are working on this, so they will not start a work again.

Describe clearly what you are doing. Be short, but detailed! Bullet points of changes, if several. Explain the physics expected change. Do not just say `Update of CxAODMaker`. 

Tick the boxes to remove your own branch once merged and to put all the commits together. 

Choose a label. If no label present, create one, and choose an appropriate color.

If the change is large, like updating a CP recommendations for a particular object, make also a GitLab issue in that repository. For CP tools, it is usually `CxAODMaker`. Then reference the GitLab issue in the MR. 

When your work is ready, and it passed the pipeline, make an explicit text tagging one of the persons allowed to merge, by saying for example 
> @abuzatu, ready to merge.

# Gitlab CI

You will create a file `gitlab-ci.yml`. You can check the syntax of your file, and what it does, by pasting its content in [here](https://gitlab.cern.ch/CxAODFramework/CxAODMaker/-/ci/lint). It seems there is a `ci/lint` for each of our packages.

To make an empty commit just to trigger the pipeline so that you can test it, do
```
git commit --allow-empty -m 'Trigger CI'
```

# Checkout Athena code locally and compile/run the framework with it using ACM

Usually you only need our code running with a particular version of `AnalysisBase`. Sometimes you may want to use for a particular `Athena` package a different version than that in `AnalysisBase`. It can be using CP tools still under development, or to explore to fix a bug, etc. This is an example from Brian Moser of how to do it. 

This is a quick guide on how to checkout athena code locally using acm within the `CxAODFramework`. This can be useful in many cases if one e.g. wants to use an older version of some tool than what is present in `AnalysisBase` or if one needs to debug dedicated tools in order for the framework to run smoothly. A step-by-step way of how to do this is sketched in the following. For more information about how to develop athena code, see [here](https://atlassoftwaredocs.web.cern.ch/gittutorial/gitlab-fork) and about ACM, see [here](https://gitlab.cern.ch/atlas-sit/acm/blob/master/README.md).

   * Go to the build directory of the framework and setup the ATLAS environment using `setupATLAS`
   * Setup `ACM` giving it the `source` directory and the `AnalysisBase` version you want to use. For example, for `21.2.12` type `acmSetup --sourcedir=../source AnalysisBase,21.2.12,here`
   * Go to the [Athena gitlab site](https://gitlab.cern.ch/atlas/athena) and create your fork of the project. If you want to later check in the code and create e.g. a merge request to the official `Athena` repository, you need to be aware of the following. Your development will be built and tested by an automated continuous integration system as you will see later. Results of these automatic builds are published on GitLab as small icons next to commit hashes. In order for this to work, the build bot needs to have access to your private fork. Therefore, please go to `Project Settings -> Members` on the top-right and add the `ATLAS` robot as a developer.
   * Now checkout the needed packages: A sparse checkout gives you only the parts of the repository you want to update. It’s great if you know you only want to make a limited set of changes and want to limit the space used by your working copy. In the build directory, do `acm sparse_clone_project athena`. This will clone your local fork of `Athena` to the source directory but not download any code yet and can take a while to complete.
   * Check in the `source/athena` directory if the original `Athena` repository is marked as upstream `git remote -v show`. If not, do `git remote add upstream https://:@gitlab.cern.ch:8443/atlas/athena.git`
   * Add packages you want to change in the build directory via `acm add_pkg athena/SomePackage`
   * Afterwards make sure the packages are put into the `CMakeList.cxx` and check if they compile via `acm find_packages && acm compile`
   * The package code is now under `source/athena/SomePackage`
   * If the code needs to be developed, check each time that you are up to date with upstream `cd ../source/athena && git fetch upstream`
   * Now create an environment that will hold the code changes in an easy accessible way - a new branch `git checkout -b master-my-topic upstream/[parent_branch] --no-track </verbatim> [parent_branch]` is the name of the branch you are developing on, e.g. `21.2; --no-track` ensures that git does not think you always want to pull push to upstream per default (might be confusing).
   * Editing and compiling afterwards with acm should now all work fine. If you are satisfied with the changes, push them to upstream and create a merge request
