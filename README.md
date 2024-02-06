## ISHMEM does not pass unitest on Borealis, Minimal-Reproducible-Example

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
/mnt/scratch/rscohn1/miniforge3/bin/cmake -B build .
/mnt/scratch/rscohn1/miniforge3/bin/cmake --build build -j --target ishmem
```

### run unitest

```shell
qsub -V -N "ishm-exp" -Wblock=true -l walltime=00:01:00,nodes=1 -j oe -o log.txt -- $I_MPI_ROOT/bin/mpiexec.hydra -n 1 \
$(pwd)/build/bin/ishmrun $(pwd)/build/ishmem-prefix/src/ishmem-build/test/unit/SHMEM/int_get_device
```

See output:
```shell
cat log.txt
```
and there is failure _Failed to acquire mapping_ and _Failed to map MR buffer_:
```
...
libfabric:148877:1707240279::cxi:mr:cxip_do_map():59<warn> x1002c1s1b0n0: ZE device memory not supported. Try disabling implicit scaling (EnableImplicitScaling=0 NEOReadDebugKeys=1).
libfabric:148877:1707240279::cxi:mr:cxip_map_cache():354<warn> x1002c1s1b0n0: Failed to acquire mapping (0xff00000000200000, 536871936): -38
libfabric:148877:1707240279::cxi:mr:cxip_regattr():1326<warn> x1002c1s1b0n0: Failed to map MR buffer: -38
[0000] WARN:  transport_ofi.c:656: ofi_mr_reg_external_heap
[0000]        fi_mr_regattr (heap) failed
[0000] WARN:  transport_ofi.c:810: allocate_recv_cntr_mr
[0000]        OFI MR registration with HMEM failed
[0000] ERROR: init.c:421: shmem_internal_heap_postinit
[0000]        Transport init failed (-38)
/mnt/scratch/lslusarc/ishmexp/build/bin/ishmrun: line 99: 148877 Aborted                 (core dumped) NEOReadDebugKeys=1 UseKmdMigration=1 numactl --cpunodebind=all $*
```
