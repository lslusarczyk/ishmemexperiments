## running on borealis

```
BQ_TAIL_INTERVAL=1 ../ci/scripts/borealis_qsub.sh 1 logs /mnt/scratch/lslusarc/venv/dds /bin/bash -c "module purge; module use -a /home/ftartagl/modulefiles; module load oneapi-testing/2024.0.0.49564.PUBLIC; module load libfabric/1.15.2.0; export I_MPI_ROOT=$ONEAPI_ROOT/mpi/latest; export export FI_CXI_OPTIMIZED_MRS=0; FI_LOG_LEVEL=Debug FI_LOG_SUBSYS=core,fabric,domain,ep_ctrl,ep_data,av,cq,eq,mr I_MPI_DEBUG=1000 SHMEM_DEBUG=1 ONEAPI_DEVICE_SELECTOR='level_zero:gpu;ext_oneapi_cuda:gpu' I_MPI_OFFLOAD=1 I_MPI_OFFLOAD_CELL_LIST=0-0 I_MPI_OFI_LIBRARY=/opt/cray/libfabric/1.15.2.0/lib64/libfabric.so.1 FI_PROVIDER_PATH=/opt/cray/libfabric/1.15.2.0/lib64 FI_PROVIDER=cxi mpirun -n 1 build/bin/ishmrun build/both"
```

informs that:
```
[0000] DEBUG: transport_ofi.c:1492: query_for_fabric
[0000]        OFI provider: cxi, fabric: cxi, domain: cxi0, mr_mode: 0x260
[0000]        max_inject: 0, max_msg: 1073741824, stx: no, stx_max: 0
libfabric:252928:1707218868::core:core:fi_fabric_():1383<info> Opened fabric: cxi
[0000] DEBUG: transport_ofi.c:1297: allocate_fabric_resources
[0000]        OFI version: built 1.15, cur. 1.15; provider version: 0.1
```
then fails with
```
libfabric:252928:1707218868::cxi:mr:cxip_do_map():59<warn> x1001c1s6b0n0: ZE device memory not supported. Try disabling implicit scaling (EnableImplicitScaling=0 NEOReadDebugKeys=1).
libfabric:252928:1707218868::cxi:mr:cxip_map_cache():354<warn> x1001c1s6b0n0: Failed to acquire mapping (0xff00000000200000, 536871936): -38
libfabric:252928:1707218868::cxi:mr:cxip_regattr():1326<warn> x1001c1s6b0n0: Failed to map MR buffer: -38
[0000] WARN:  transport_ofi.c:656: ofi_mr_reg_external_heap
[0000]        fi_mr_regattr (heap) failed
[0000] WARN:  transport_ofi.c:810: allocate_recv_cntr_mr
[0000]        OFI MR registration with HMEM failed
[0000] ERROR: init.c:421: shmem_internal_heap_postinit
[0000]        Transport init failed (-38)
build/bin/ishmrun: line 99: 252928 Aborted                 (core dumped) NEOReadDebugKeys=1 UseKmdMigration=1 numactl --cpunodebind=all $*
```
