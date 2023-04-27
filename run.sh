#!/bin/env bash

if (command -v module &> /dev/null)
then
module load cmake gcc openmpi/4.1.0 boost
fi

set -x;

# rm *.txt ; cmake -B build-graph-cart -S graph-cart && cmake --build build-graph-cart && mpirun --use-hwthread-cpus -np 4 ./build-graph-cart/bin/stencil_mpi 6 4 5
# rm -f *.txt;
# --use-hwthread-cpus
# --mca osc ucx
# --mca osc pt2pt // ethernet

# https://www.open-mpi.org/faq/?category=openfabrics#ib-components
# --mca osc pt2pt \

# --mca osc ucx \
# --mca pml ucx \
# --mca scoll ucx \
# --mca atomic ucx \

# --mca pml ucx -mca osc ucx

# cmake -B build-horizontal-split \
# -DCMAKE_BUILD_TYPE=Release \
# -DPRINT_PERF=FALSE \
# -DOPENMP:BOOL=TRUE \
# -S horizontal-split && \
# cmake --build build-horizontal-split

# --mca osc pt2pt \
# --mca btl openib,self,vader
# --mca mpi_leave_pinned 1
# --mca coll_fca_enable 1

# --mca osc pt2pt \
# --mca orte_base_help_aggregate false \
# --mca btl_openib_allow_ib true \
# --mca opal_common_ucx_opal_mem_hooks true \

# --mca osc ucx \
# --mca pml ucx \

# export EXTRAE_HOME=/mnt/beegfs/workdir/kevin.barre/extrae/extrae/installdir
# export LD_PRELOAD=${EXTRAE_HOME}/lib/libmpitrace.so
# export EXTRAE_CONFIG_FILE=./extrae-mpi.xml

# export LD_PRELOAD=${EXTRAE_HOME}/lib/libompitrace.so
# export EXTRAE_CONFIG_FILE=./extrae-mpi-omp.xml
#  LD_PRELOAD=path1:path2

export OMP_SCHEDULE=static
export OMP_PROC_BIND=TRUE
export OMP_PLACES=threads
export OMP_NUM_THREADS=20

# --single
# hwloc-bind --physical socket:0.pu:7
time mpirun \
--mca osc pt2pt \
--mca orte_base_help_aggregate false \
--mca btl_openib_allow_ib true \
--mca opal_common_ucx_opal_mem_hooks true \
-np ${1} \
./build-horizontal-split/bin/stencil \
--nbr_of_column 80000 \
--nbr_of_row 80000 \
--nbr_iters 10000 \
--ompthread_nbr ${OMP_NUM_THREADS} \
--init_val 1