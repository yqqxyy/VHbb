ps -x | grep runRepeatedlyRucioGetJobs | grep bash | awk '{print $1}' | while read -r PID; do kill -9 ${PID}; done
ps -x | grep count_Nentry_SumOfWeight | awk '{print $1}' | while read -r PID; do kill -9 ${PID}; done
ps -x | grep rucio_get_jobs | awk '{print $1}' | while read -r PID; do kill -9 ${PID}; done
ps -x | grep rucio | grep download | awk '{print $1}' | while read -r PID; do kill -9 ${PID}; done
ps -x | grep sleep | awk '{print $1}' | while read -r PID; do kill -9 ${PID}; done
