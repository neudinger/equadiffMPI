# Distributed diffusion equation C/C++ OpenMP one-sided communication MPI

![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black) ![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)

![OpenMPI](https://img.shields.io/badge/OpenMPI-%23f01742.svg?style=for-the-badge&logoColor=white) ![HWLOC](https://img.shields.io/badge/HWLOC-%23f01742.svg?style=for-the-badge&logoColor=white) ![OMP](https://img.shields.io/badge/OpenMp-%23316192.svg?style=for-the-badge)


> - Cartesian split
> - Horizontal split

Hybrid computation with multiple parallel computation level :

(Cluster level) -> (Machine level) -> (CPU Level) -> (Core Level)

- Distributed parallel operations layer with MPI domain splitting.
- Multithreaded parallel operations layer with OpenMP.
- Vectorized parallel operations layer with cached blocked loops.

Usage of [HWLOC](https://www.open-mpi.org/projects/hwloc/) to gather hierarchical topology and specified thread core process binding.

Boost was used for program_options.
The cmake will download build program_options if boost is not found.
Only this library will be linked to reduce the library loading overhead.

## Cartesian split

```bash
rm -rf build-cartesian-split/ && \
cmake -B build-cartesian-split \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DPRINT_PERF=FALSE \
-DOPENMP:BOOL=TRUE -S cartesian-split && \
cmake --build build-cartesian-split
```

## Horizontal split

Build :

```bash
rm -rf build-horizontal-split/ && \
cmake -B build-horizontal-split \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DPRINT_PERF=FALSE \
-DOPENMP:BOOL=TRUE -S horizontal-split && \
cmake --build build-horizontal-split
```

Run :

```bash
mpirun \
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
```


---

C++ 17 was used due to usage of string_view and initializer_list.\
MPI for distributed layer.\
OpenMP for multithreading purpose.

This project was built with slurm as cluster node scheduler.
