#!/usr/bin/python

def parse_derivation_dataset(i_sample,verbose=True,debug=False):
    if debug:
        print "Start parse_derivation_dataset("+i_sample+")"
    # if the last element is /, remove it
    if i_sample[-1]=='/':
        i_sample=i_sample[0:-1]
    if debug:
        print "i_sample",i_sample
    # do it here in a new way from Adrian, without offsets
    list_i_sample=i_sample.split(".")
    # both MC and data have the same structure, for example
    # data
    # i_sample:                     data17_13TeV.00340453.physics_Main.deriv.DAOD_HIGG5D1.f894_m1902_p3402
    # sample_out:  group.phys-higgs.data17_13TeV.00340453.physics_Main.f894_m1902_p3402.HIGG5D1.30-02/ 
    # MC:
    # i_sample:                     mc16_13TeV.363358.Sherpa_221_NNPDF30NNLO_WqqZll.deriv.DAOD_HIGG5D1.e5525_s3126_r9781_r9778_p3371
    # sampe_out:   group.phys-higgs.mc16_13TeV.363358.Sh_221_WqqZll.e5525_s3126_r9781.HIGG5D1.30-02-3/
    if debug:
        print "list_i_sample",list_i_sample
    # from the examples we see that there are six elements in the list, so we enforce this check
    assert(len(list_i_sample)==6)
    sampleType       = list_i_sample[0] # data17_13TeV or mc16_13TeV
    sampleRun        = list_i_sample[1] # 00340453 or 363358
    sampleText       = list_i_sample[2] # physics_Main or Sherpa_221_NNPDF30NNLO_WqqZll
    sampleStage      = list_i_sample[3] # deriv (same for data or MC)
    sampleDerivation = list_i_sample[4] # DAOD_HIGG5D1 (same for data or MC)
    sampleAMITag     = list_i_sample[5] # f894_m1902_p3402 or e5525_s3126_r9781_r9778_p3371
    if debug:
        print "%-25s %-40s" % ("sampleType",      sampleType)
        print "%-25s %-40s" % ("sampleRun",       sampleRun)
        print "%-25s %-40s" % ("sampleText",      sampleText)
        print "%-25s %-40s" % ("sampleStage",     sampleStage)
        print "%-25s %-40s" % ("sampleDerivation",sampleDerivation)
        print "%-25s %-40s" % ("sampleAMITag",    sampleAMITag)
    # make the Text and AMI tags shorter (prune), to be consistent with what is done in hsg5framework.cxx
    if "data" in sampleType:
        if debug:
            print "data sample, so do not prune Text and AMITag, as already short."
        sampleTextPruned=sampleText
        sampleAMITagPruned=sampleAMITag
    elif "mc" in sampleType:
        # start prun Text
        if debug:
            print "mc sample, so  make the text shorter (prune), to be consistent with what is done in hsg5framework.cxx"
        sampleTextPruned=sampleText
        sampleTextPruned=sampleTextPruned.replace("ParticleGenerator","PG")
        sampleTextPruned=sampleTextPruned.replace("Pythia","Py")
        sampleTextPruned=sampleTextPruned.replace("Powheg","Pw")
        sampleTextPruned=sampleTextPruned.replace("MadGraph","MG")
        sampleTextPruned=sampleTextPruned.replace("EvtGen","EG")
        sampleTextPruned=sampleTextPruned.replace("Sherpa","Sh")
        sampleTextPruned=sampleTextPruned.replace("NNPDF30NNLO_","")
        sampleTextPruned=sampleTextPruned.replace("Filter","f")
        sampleTextPruned=sampleTextPruned.replace("Veto","v")
        sampleTextPruned=sampleTextPruned.replace("MAXHTPTV","M")
        sampleTextPruned=sampleTextPruned.replace("_CT10","")
        # done prune Text
        # start prune AMITag
        if debug:
            print "mc sample, so prune ami tag to keep the e-tags, the s-tags and first r-tag,  to be consistent with what is done in hsg5framework.cxx"
        sampleAMITagPruned=""
        list_sampleAMITag=sampleAMITag.split("_")
        count_tag_to_write=0
        count_rtag=0
        for sampleAMITagElement in list_sampleAMITag:
            if debug:
                print "sampleAMITagElement",sampleAMITagElement
            if sampleAMITagElement[0]=='r':
                count_rtag+=1
            if sampleAMITagElement[0]=='e' or sampleAMITagElement[0]=='s' or count_rtag==1:
                count_tag_to_write+=1
                if count_tag_to_write>1:
                    sampleAMITagPruned+="_"
                # done if
                sampleAMITagPruned+=sampleAMITagElement
            # done if
        # done for
    # done for prune AMITag
    else:
        print "The sampleType",sampleType,"is neither data or mc. Something bizarre happened. Will ABORT!!!"
        assert(False)
    # done if data or MC
    if debug:
        print "Pruning text:"
        print "%-25s %-40s" % ("sampleTextPruned", sampleTextPruned)
        print "%-25s %-40s" % ("sampleText", sampleText)
        print "Pruning AMITag:"
        print "%-25s %-40s" % ("sampleAMITagPruned", sampleAMITagPruned)
        print "%-25s %-40s" % ("sampleAMITag", sampleAMITag)
    # make the derivation shorter (prune), to be consistent with what is done in hsg5framework.cxx
    # from DAOD_HIGG5D1 keep only HIGG5D1
    sampleDerivationPruned=sampleDerivation.split("_")[1]
    if debug:
        print "Pruning Derivation:"
        print "%-25s %-40s" % ("sampleDerivationPruned", sampleDerivationPruned)
        print "%-25s %-40s" % ("sampleDerivation", sampleDerivation)
    # compute result
    result=(sampleType,sampleRun,sampleText,sampleStage,sampleDerivation,sampleAMITag,sampleTextPruned,sampleAMITagPruned,sampleDerivationPruned)
    if debug:
        print "result",result
    return result
# done function

def get_sample_out(user,sampleType,sampleRun,sampleTextPruned,sampleAMITagPruned,sampleDerivationPruned,vtag,verbose=True,debug=False):
    sample_out=user+"."+sampleType+"."+sampleRun+"."+sampleTextPruned+"."+sampleAMITagPruned+"."+sampleDerivationPruned+"."+vtag
    if debug:
        print "Final:"
    if debug or verbose:
        print "sample_out",sample_out
    # check length
    full_name = sample_out+"_CxAOD.root"
    max_str = 132
    if len(full_name) >= max_str:
        print "WARNING : "+full_name+" output name is "+str(len(full_name))+" chars, it will crash the job (should be <"+str(max_str)+"). Re-visit the name structure."
    return sample_out
# done function
