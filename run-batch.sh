#!/bin/env bash

set -x

echo """
SLURM_JOB_NAME, \"${SLURM_JOB_NAME}\", PBS_JOBNAME Name of the job
SLURM_JOB_ID, \"${SLURM_JOB_ID}\", PBS_JOBID The ID of the job allocation
SLURM_JOB_NODELIST, \"${SLURM_JOB_NODELIST}\", PBS_NODEFILE List of nodes allocated to the job
SLURM_TASK_PID, \"${SLURM_TASK_PID}\", The process ID of the task being started
SLURMD_NODENAME, \"${SLURMD_NODENAME}\", Name of the node running the job script
SLURM_JOB_NUM_NODES, \"${SLURM_JOB_NUM_NODES}\", Total number of different nodes in the job's resource allocation
SLURM_SUBMIT_DIR, \"${SLURM_SUBMIT_DIR}\", PBS_O_WORKDIR The directory from which sbatch was invoked
SLURM_SUBMIT_HOST, \"${SLURM_SUBMIT_HOST}\", PBS_O_HOST The hostname of the computer from which sbatch was invoked
SLURM_CPUS_ON_NODE, \"${SLURM_CPUS_ON_NODE}\", Number of CPUS on the allocated node
SLURM_NNODES, \"${SLURM_NNODES}\", Total number of nodes in the job's resource allocation
SLURM_MEM_PER_CPU, \"${SLURM_MEM_PER_CPU}\", Same as --mem-per-cpu
SLURM_NPROCS, \"${SLURM_NPROCS}\", SLURM_NPROCS PBS_NUM_NODES Same as -n, --ntasks
SLURM_JOB_CPUS_PER_NODE, \"${SLURM_JOB_CPUS_PER_NODE}\", PBS_NUM_PPN Count of processors available to the job on this node.
SLURM_CLUSTER_NAME, \"${SLURM_CLUSTER_NAME}\", Name of the cluster on which the job is executing
SLURM_JOB_ACCOUNT, \"${SLURM_JOB_ACCOUNT}\", Account name associated of the job allocation
SLURM_NTASKS_PER_NODE, \"${SLURM_NTASKS_PER_NODE}\", Number of tasks requested per node. Only set if the --ntasks-per-node option is specified.
SLURM_NTASKS_PER_SOCKET, \"${SLURM_NTASKS_PER_SOCKET}\", Number of tasks requested per socket. Only set if the --ntasks-per-socket option is specified.
SLURM_JOB_GPUS, \"${SLURM_JOB_GPUS}\", GPU IDs allocated to the job (if any).
SLURM_CPUS_PER_TASK, \"${SLURM_CPUS_PER_TASK}\", --cpus-per-task=<ntpt>
SLURM_TASKS_PER_NODE, \"${SLURM_TASKS_PER_NODE}\", Number of tasks to be initiated on each node
"""

if [ -z "$OMP_NUM_THREADS" ];
then
    export OMP_NUM_THREADS=1
    NODES=${SLURM_NPROCS}
else
    NODES=${SLURM_JOB_NUM_NODES}
fi

# https://github.com/open-mpi/ompi/issues/6517
# export OMPI_MCA_opal_common_ucx_opal_mem_hooks=1
# export OMPI_MCA_pml_ucx_verbose=100

# --mca osc ucx \
# --mca pml ucx \
# --mca scoll ucx \
# --mca atomic ucx \

time mpirun \
--mca osc pt2pt \
--mca orte_base_help_aggregate 0 \
--mca btl_openib_allow_ib 1 \
--mca opal_common_ucx_opal_mem_hooks 1 \
-np ${NODES} \
./build-horizontal-split/bin/stencil \
--ompthread_nbr ${OMP_NUM_THREADS} \
--nbr_of_column ${1} \
--nbr_of_row ${1} \
--nbr_iters ${2} \
--init_val 1
