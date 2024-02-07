## ISHMEM with MPI Minimal Example for experiments

### checkout example 

Log into Borealis access node

```shell
git clone -b mpi_with_ishm_on_borealis git@github.com:lslusarczyk/ishmemexperiments.git ishmemexperiments
cd ishmemexperiments
```

### prepare environment
Simple script configure environment, that is loads newest IMPI and crays fabric.
Calls required exports. Sets extra verbose logs. See oneapi.sh script source code for details.
```shell
source oneapi.sh
```

### build ishmem and its unittests
See CMakeLists.txt source code for exact configure commands.
They follow instruction from ISHMEM documentation. 
```shell
/mnt/scratch/rscohn1/miniforge3/bin/cmake -B build \
-DMPIEXEC_EXECUTABLE=$I_MPI_ROOT/bin/mpiexec.hydra \
-DMPI_C_COMPILER=$I_MPI_ROOT/bin/mpicc -DMPI_CXX_COMPILER=$I_MPI_ROOT/bin/mpicxx .
/mnt/scratch/rscohn1/miniforge3/bin/cmake --build build -j --target ishmem
/mnt/scratch/rscohn1/miniforge3/bin/cmake --build build -j --target alltests
```

### run unitest

```shell
qsub -V -N "ishm-exp" -Wblock=true -l walltime=00:01:00,nodes=1 -j oe -o log.txt -- $I_MPI_ROOT/bin/mpiexec.hydra -n 1 \
$(pwd)/build/bin/ishmrun $(pwd)/build/ishmem-prefix/src/ishmem-build/test/unit/SHMEM/int_get_device
```

### run example

```shell
qsub -V -N "ishm-exp" -Wblock=true -l walltime=00:01:00,nodes=1 -j oe -o log.txt -- $I_MPI_ROOT/bin/mpiexec.hydra -n 1 \
$(pwd)/build/bin/ishmrun $(pwd)/build/both
```

See output:
```shell
cat log.txt
```
