#!/bin/bash
# Adrian Buzatu (adrian.buzatu@cern.ch) on behalf of the CxAODFramework group

NUMBER_REPEAT=$1
SLEEP_TIME_IN_SECONDS=$2

# run repeateadly, with pause in between
for i in $(seq 1 1 ${NUMBER_REPEAT})
do
    echo ""
    echo "*********************"
    echo "** Running i=${i} ***"
    echo "*********************"
   # download automatically for first step and before each other step
   if [[ ${i} == 1 ]]; then
       command="sleep 1 && ./count_Nentry_SumOfWeight.py 1 0 0 && ./checkYields.py"
   elif [[ ${i} == 2 ]]; then
       command="sleep ${SLEEP_TIME_IN_SECONDS}"
   else
       command="sleep ${SLEEP_TIME_IN_SECONDS}"
   fi
   echo ${command}
   eval ${command}
   # download in each folder one at a time what is new
   COMMAND="./rucio_get_jobs.py out*"
   echo "COMMAND=${COMMAND}"
   eval ${COMMAND}
   echo "Done step i=${i}."
done

echo ""
echo "**************************************"
echo "** Done all ${NUMBER_REPEAT} steps ***"
echo "**************************************"
command="./count_Nentry_SumOfWeight.py 0 0 1 && ./checkYields.py && grep -v 1.000 dslist_NevtDxAOD_yield.txt"
echo ${command}
eval ${command}

echo ""
echo ""
echo "Done runRepeatedlyRucioGetJobs.sh in folder ${PWD}."

