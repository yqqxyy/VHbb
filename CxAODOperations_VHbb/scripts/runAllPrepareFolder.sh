#!/bin/bash
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group

echo "Start runAllPrepareFolder.sh!"

# settings on how to run
DO_LOG="1"

# settings of what to run
DO_COPY_ALL_SCRIPTS_FROM_FRAMEWORK="0"
STEM_INITIAL="mcdata"
DO_COPY_SAMPLE_INFO_FROM_FRAMEWORK="0"
DO_COPY_SAMPLE_INFO_TO_FRAMEWORK="0"
DO_UPDATE_OUTPUT_SAMPLE_LIST_VTAG_WITH_SED="0"
VTAG_INITIAL="32-07"
VTAG_FINAL="32-07-6"
DO_UPDATE_OUTPUT_SAMPLE_LIST_FROM_FRAMEWORK="0"
DO_UPDATE_OUTPUT_SAMPLE_LIST_FROM_ANOTHER="0"
STEM_REPLACEMENT="mcdata2"
DO_MOVE_CXAOD_FROM_PREVIOUS_FOLDER="0"
PREVIOUS_FOLDER="/data06/abuzatu/data/CxAOD/190119_32-07"
DO_RUCIO_GET_FROM_GRID="0"
NUMBER_REPEAT="1"
SLEEP_TIME_IN_SECONDS="3600"
DO_COUNT_YIELDS_FOR_AMI="0"
DO_COUNT_YIELDS_FOR_READER="0"
DO_CHECK_YIELDS="0"
DO_COPY_TO_LOCAL="0"
LOCAL_CXAOD_FOLDER="/data06/abuzatu/data/CxAOD/ToUseInReader"
DO_COPY_TO_EOS="0"
DO_REPLICATE_TO_GRID="0"

# calculate here the names of folders you want to download
A=$(ls -d prepare_*)
STEMs=""
for B in ${A}
do
    # remove prepare_ from B
    C=${B##*prepare_}
    STEMs+=" ${C}"
done
# you can overwrite here if you want to run only one or only a few, for example
# STEMs="0L_a_32-07 0L_d_32-07 0L_e_32-07 1L_a_32-07 1L_d_32-07 1L_e_32-07 2L_a_32-07 2L_d_32-07 2L_e_32-07 1L_e_32-07-P"
# STEMs="0L_a_32-07"
# STEMs="1L_e_32-07-P"
echo "STEM=${STEMs}"

# do for each folder one at a time what is new
for STEM in ${STEMs}
do
    #
    if [[ ${STEM} == "0L_"* ]]; then
	echo "STEM=${STEM} is 0L"
	# STEM_INITIAL="signals"
    fi
    #
    if [[ ${STEM} == "1L_"* ]]; then
	echo "STEM=${STEM} is 1L"
	# STEM_INITIAL="signalsdata"
    fi
    #
    if [[ ${STEM} == "2L_"* ]]; then
	echo "STEM=${STEM} is 2L"
	# STEM_INITIAL="signals"
    fi

    #
    FOLDER="prepare_${STEM}"
    echo "Start FOLDER=${FOLDER}"
    cd ${FOLDER}
    pwd

    # create a backup with a time stamp
    TIME_STAMP="$(date +%Y%m%d_%H%M%S)"

    # update the line below with your own settings and uncomment if you want to copy to eos as soon as downloaded from grid
    # careful, this is not automated yet to pick up the correct name for each FOLDER, so the copy to eos works only for one FOLDER
    # when you hard code the correct path, as below
    CHANNEL=$(echo $STEM | cut -d'_' -f1) # e.g. 0L
    PERIOD=$(echo $STEM | cut -d'_' -f2) # e.g. d
    VTAG=$(echo $STEM | cut -d'_' -f3) # e.g. 181009
    echo "CHANNEL=${CHANNEL} PERIDO=${PERIOD} VTAG=${VTAG}"
    if [[ ${CHANNEL} == "0L" ]]; then
        DERIVATION="HIGG5D1"
    elif [[ ${CHANNEL} == "1L" ]]; then
        DERIVATION="HIGG5D2"
    elif [[ ${CHANNEL} == "2L" ]]; then
        DERIVATION="HIGG2D4"
    else
        echo "CHANNEL=${CHANNEL} is not known, so will ABORT!!!"
        exit -1
    fi

    # deal with output sample lists, there are several actions we can take
    OUTPUT_SAMPLE_LIST="out_sample_list_sample_grid.13TeV_25ns.${STEM_INITIAL}_${PERIOD}.${DERIVATION}.txt"
    echo "OUTPUT_SAMPLE_LIST=${OUTPUT_SAMPLE_LIST}"
    FRAMEWORK_FOLDER_OPERATIONS="$WorkDir_DIR/../../source/CxAODOperations_VHbb"
    FRAMEWORK_FOLDER_FOR_OUTPUT_SAMPLE_LIST="${FRAMEWORK_FOLDER_OPERATIONS}/data/CxAOD/CxAOD_r${VTAG}/"
    echo "FRAMEWORK_FOLDER_FOR_OUTPUT_SAMPLE_LIST=${FRAMEWORK_FOLDER_FOR_OUTPUT_SAMPLE_LIST}"
    mkdir -p ${FRAMEWORK_FOLDER_FOR_OUTPUT_SAMPLE_LIST}
    #
    DS_LIST_INITIAL="dslist_NevtDxAOD.txt"
    DS_LIST_FRAMEWORK="dslist_NevtDxAOD.${STEM_INITIAL}_${PERIOD}.${DERIVATION}.txt"
    #
    echo "Start doing things for FOLDER=${FOLDER}"

    # 
    if [[ ${DO_COPY_ALL_SCRIPTS_FROM_FRAMEWORK} == "1" ]]; then
	echo "Creating a backup of all scripts in framework (*.sh, *.py with the TIME_STAMP=${TIME_STAMP}, then copying locally"
	# update scripts
	FILE_NAMEs="createOutputLisFromAlreadyDownloaded.sh,operatePandaJobs.sh,runRepeatedlyRucioGetJobs.sh,checkYields.py,copy_CxAODs_to_eos.py,count_Nentry_SumOfWeight.py,replicateToGrid.py,rucio_get_jobs.py"
	# FILE_NAME=""
	for FILE_NAME in `echo "${FILE_NAMEs}" | awk -v RS=, '{print}'`
	do
	    COMMAND="cp ${FILE_NAME} bk_${TIME_STAMP}_${FILE_NAME} && cp ${FRAMEWORK_FOLDER_OPERATIONS}/scripts/${FILE_NAME} ."
	    echo "COMMAND=${COMMAND}"
	    eval ${COMMAND}
	done
    fi

    # 
    if [[ ${DO_COPY_SAMPLE_INFO_FROM_FRAMEWORK} == "1" ]]; then
	# output sample list
	echo "Creating a backup of ${OUTPUT_SAMPLE_LIST} with the TIME_STAMP=${TIME_STAMP}"
	cp ${OUTPUT_SAMPLE_LIST} bk_${TIME_STAMP}_${OUTPUT_SAMPLE_LIST}
	# now do the real copy
	COMMAND="cp ${FRAMEWORK_FOLDER_FOR_OUTPUT_SAMPLE_LIST}/${OUTPUT_SAMPLE_LIST} ."
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
	#
	# dslist_NevtDxAOD.txt (with AMI values)
	echo "Creating a backup of ${DS_LIST_INITIAL} with the TIME_STAMP=${TIME_STAMP}"
	cp ${DS_LIST_INITIAL} bk_${TIME_STAMP}_${DS_LIST_INITIAL}
	# now do the real copy
	COMMAND="cp ${FRAMEWORK_FOLDER_FOR_OUTPUT_SAMPLE_LIST}/${DS_LIST_FRAMEWORK} ${DS_LIST_INITIAL}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
	# update sample_info.txt
	# 
	FILE_NAMEs="sample_info.txt"
	# FILE_NAME=""
	for FILE_NAME in `echo "${FILE_NAMEs}" | awk -v RS=, '{print}'`
	do
	    COMMAND="cp ${FILE_NAME} bk_${TIME_STAMP}_${FILE_NAME} && cp ${FRAMEWORK_FOLDER_OPERATIONS}/data/DxAOD/info/${FILE_NAME} ."
	    echo "COMMAND=${COMMAND}"
	    eval ${COMMAND}
	done
    fi

    # 
    if [[ ${DO_COPY_SAMPLE_INFO_TO_FRAMEWORK} == "1" ]]; then
	# output sample list
	COMMAND="cp ${OUTPUT_SAMPLE_LIST} ${FRAMEWORK_FOLDER_FOR_OUTPUT_SAMPLE_LIST}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
	# dslist_NevtDxAOD.txt (with AMI values)
	COMMAND="cp ${DS_LIST_INITIAL} ${FRAMEWORK_FOLDER_FOR_OUTPUT_SAMPLE_LIST}/${DS_LIST_FRAMEWORK}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
	#
	FILE_NAMEs="sample_info.txt"
	# FILE_NAME=""
	for FILE_NAME in `echo "${FILE_NAMEs}" | awk -v RS=, '{print}'`
	do
	    COMMAND="cp ${FILE_NAME} ${FRAMEWORK_FOLDER_OPERATIONS}/data/DxAOD/info/."
	    echo "COMMAND=${COMMAND}"
	    eval ${COMMAND}
	done
    fi

    #
    if [[ ${DO_UPDATE_OUTPUT_SAMPLE_LIST_VTAG_WITH_SED} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_PREFIX="nohup"
	    LOG_SUFFIX=" >& ../run_update_output_sample_list_vtag_with_sed_${STEM}.log &"
	else
            LOG_PREFIX=""
	    LOG_SUFFIX=""
	fi
	INITIAL="${OUTPUT_SAMPLE_LIST}"
	echo "backup ${INITIAL} before replacing the vtag with sed"
	cp ${INITIAL} bk_${TIME_STAMP}_${INITIAL}
	# in case you need to recover from the initial bk
	# mv bk_*_${INITIAL} ${INITIAL}
	# in case you ran with 32-07 but you needed in fact 32-07-3
	# sed -i -e 's|32-07|32-07-3|g' ${INITIAL}
	COMMAND="sed -i -e 's|${VTAG_INITIAL}|${VTAG_FINAL}|g' ${INITIAL}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi

    #
    if [[ ${DO_UPDATE_OUTPUT_SAMPLE_LIST_FROM_FRAMEWORK} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_PREFIX="nohup"
	    LOG_SUFFIX=" >& ../run_update_output_sample_list_from_framework_${STEM}.log &"
	else
            LOG_PREFIX=""
	    LOG_SUFFIX=""
	fi
	INITIAL="${OUTPUT_SAMPLE_LIST}"
	REPLACEMENT="${FRAMEWORK_FOLDER_FOR_OUTPUT_SAMPLE_LIST}/${OUTPUT_SAMPLE_LIST}"
	COMMAND="${LOG_PREFIX} ../updateOutputSampleList.sh ${INITIAL} ${REPLACEMENT} ${LOG_SUFFIX}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi

    #
    if [[ ${DO_UPDATE_OUTPUT_SAMPLE_LIST_FROM_ANOTHER} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_PREFIX="nohup"
	    LOG_SUFFIX=" >& ../run_update_output_sample_list_from_another_${STEM}.log &"
	else
            LOG_PREFIX=""
	    LOG_SUFFIX=""
	fi
	INITIAL="${OUTPUT_SAMPLE_LIST}"
	# assuming you copy those with mcdata2 in the main folder, that has the prepare_ folders
	REPLACEMENT="../out_sample_list_sample_grid.13TeV_25ns.${STEM_REPLACEMENT}_${PERIOD}.${DERIVATION}.txt"
	ls -lh ${INITIAL}
	ls -lh ${REPLACEMENT}
	COMMAND="${LOG_PREFIX} ../updateOutputSampleList.sh ${INITIAL} ${REPLACEMENT} ${LOG_SUFFIX}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi

    #
    if [[ ${DO_MOVE_CXAOD_FROM_PREVIOUS_FOLDER} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_PREFIX="nohup"
	    LOG_SUFFIX=" >& ../run_move_CxAOD_from_previous_folder_${STEM}.log &"
	else
            LOG_PREFIX=""
	    LOG_SUFFIX=""
	fi
	# 
	# COMMAND="${LOG_PREFIX} ls -lh ${PREVIOUS_FOLDER}/${FOLDER}/*_CxAOD.root {PREVIOUS_FOLDER}/${FOLDER}/*_hist ${LOG_SUFFIX}"
	COMMAND="${LOG_PREFIX} mv ${PREVIOUS_FOLDER}/${FOLDER}/*_CxAOD.root ${PREVIOUS_FOLDER}/${FOLDER}/*_hist . ${LOG_SUFFIX}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi

    #
    if [[ ${DO_RUCIO_GET_FROM_GRID} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_PREFIX="nohup"
	    LOG_SUFFIX=" >& ../run_rucio_${STEM}.log &"
	else
	    LOG_PREFIX=""
	    LOG_SUFFIX=""
	fi
        COMMAND="${LOG_PREFIX} ./runRepeatedlyRucioGetJobs.sh ${NUMBER_REPEAT} ${SLEEP_TIME_IN_SECONDS} ${LOG_SUFFIX}"
	pwd
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi

    # 
    if [[ ${DO_COUNT_YIELDS_FOR_AMI} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_SUFFIX=" >& ../run_count_yields_for_ami_${STEM}.log &"
	else
	    LOG_SUFFIX=""
	fi
	COMMAND="./count_Nentry_SumOfWeight.py 1 0 0 && ./checkYields.py && grep -v 1.000 dslist_NevtDxAOD_yield.txt ${LOG_SUFFIX}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi

    # 
    if [[ ${DO_COUNT_YIELDS_FOR_READER} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_SUFFIX=" >& ../run_count_yields_for_reader_${STEM}.log &"
	else
	    LOG_SUFFIX=""
	fi
	COMMAND="./count_Nentry_SumOfWeight.py 0 0 1 ${LOG_SUFFIX}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi

    # 
    if [[ ${DO_CHECK_YIELDS} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_SUFFIX=" >& ../run_check_yields_${STEM}.log &"
	else
	    LOG_SUFFIX=""
	fi
	COMMAND="grep -v 1.000 dslist_NevtDxAOD_yield.txt ${LOG_SUFFIX}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi

    #
    if [[ ${DO_COPY_TO_LOCAL} == "1" ]]; then
	echo "DO_COPY_TO_LOCAL"
	# when copying to local (implicitely via symbolic links) we want to remove the initial folder and create it again
	LOCAL_FINAL_CXAOD_FOLDER="${LOCAL_CXAOD_FOLDER}/${DERIVATION}_13TeV/CxAOD_${VTAG}_${PERIOD}"
	COMMAND="echo LOCAL_FINAL_CXAOD_FOLDER=${LOCAL_FINAL_CXAOD_FOLDER} && rm -rf  ${LOCAL_FINAL_CXAOD_FOLDER}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
	# do the actual copy
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_PREFIX="nohup"
	    LOG_SUFFIX=" >& ../run_copy_to_local_${STEM}.log &"
	else
            LOG_PREFIX=""
	    LOG_SUFFIX=""
	fi
	COMMAND="${LOG_PREFIX} ./copy_CxAODs_to_eos.py 0 1 1 1 ${DERIVATION} ${VTAG}_${PERIOD} ${LOCAL_CXAOD_FOLDER}/ ${LOG_SUFFIX}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi
    
    # 
    if [[ ${DO_COPY_TO_EOS} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_PREFIX="nohup"
	    LOG_SUFFIX=" >& ../run_copy_to_eos_${STEM}.log &"
	else
            LOG_PREFIX=""
	    LOG_SUFFIX=""
	fi
	PWD=$(pwd)
	echo "PWD=${PWD}"
        USERNAME=$USER
        echo "User=${USERNAME}"
        MAKE_SYMBOLIC_LINK_WHEN_EOS_AS_DESTINATION="0"
        PATH_OF_EOS_DESTINATION="/eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r${VTAG}/"
	if [[ ${PWD} == *"eos"*${USERNAME}* ]]; then
	    echo "Copying from personal EOS."
	    if [[ ${PATH_OF_EOS_DESTINATION} != *"/eos/atlas/atlascerngroupdisk/phys-higgs"* ]]; then
		echo "Copying only SYMBOLIC links"
                MAKE_SYMBOLIC_LINK_WHEN_EOS_AS_DESTINATION="1"
	    fi
	else
	    echo "Either not copying from eos, or in eos and not copying to Higgs eos"
	    MAKE_SYMBOLIC_LINK_WHEN_EOS_AS_DESTINATION="0"
	fi
        # example: ./copy_CxAODs_to_eos.py 0 1 1 0 HIGG5D1    32-02_a /eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/VH/CxAOD_r32-07/  
        COMMAND="${LOG_PREFIX} ./copy_CxAODs_to_eos.py 0 1 1 ${MAKE_SYMBOLIC_LINK_WHEN_EOS_AS_DESTINATION} ${DERIVATION} ${VTAG}_${PERIOD} ${PATH_OF_EOS_DESTINATION} ${LOG_SUFFIX}"
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi
    
    #
    if [[ ${DO_REPLICATE_TO_GRID} == "1" ]]; then
	if [[ ${DO_LOG} == "1" ]]; then
	    LOG_PREFIX="nohup"
	    LOG_SUFFIX=" >& ../run_replicate_to_grid_${STEM}.log &"
	else
	    LOG_PREFIX=""
	    LOG_SUFFIX=""
	fi
	RUCIO_ACCOUNT="phys-higgs"
        COMMAND="${LOG_PREFIX} ./replicateToGrid.py physgroup=${RUCIO_ACCOUNT} all ${OUTPUT_SAMPLE_LIST} ${LOG_SUFFIX}"
	pwd
	echo "COMMAND=${COMMAND}"
	eval ${COMMAND}
    fi
    
    #
    cd ..
done # done for loop over STEMs

echo ""
echo ""
echo "Done all runAllPrepareFolder.sh!"
