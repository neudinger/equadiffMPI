#!/bin/env bash

declare here="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
declare worker=20
declare nodes=8
declare ntasksOnNodes=2
declare ntasks=`expr ${ntasksOnNodes} \* ${nodes}`
ntasks=`expr ${ntasks} \* ${worker}`
ntasksOnNodes=`expr ${ntasksOnNodes} \* ${worker}`

set -x;

# rm -rf build-horizontal-split/ && \
# cmake -B build-horizontal-split \
# -DCMAKE_BUILD_TYPE=RelWithDebInfo \
# -DPRINT_PERF=FALSE \
# -DOPENMP:BOOL=TRUE -S horizontal-split && \
# cmake --build build-horizontal-split
# https://slurm.schedmd.com/mc_support.html
# --cpu-bind=
# 95GB

sbatch \
--account "" \
--cpus-per-task=1 \
--mem-per-cpu=90gb \
--nodes=${nodes} \
--ntasks=${ntasks} \
--ntasks-per-node=${ntasksOnNodes} \
--partition=cpu_dist \
--time=24:00:00 \
${here}/run.sh ${ntasks}