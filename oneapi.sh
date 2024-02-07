module purge
module use -a /home/ftartagl/modulefiles
module load oneapi-testing/2024.1.0.339.PUBLIC
module load libfabric/1.15.2.0

export I_MPI_ROOT=$ONEAPI_ROOT/mpi/latest

export FI_CXI_OPTIMIZED_MRS=0
export FI_PROVIDER=cxi
export FI_LOG_LEVEL=Debug
export FI_LOG_SUBSYS=core,fabric,domain,ep_ctrl,ep_data,av,cq,eq,mr
export I_MPI_DEBUG=1000
export SHMEM_DEBUG=1
export I_MPI_OFFLOAD=1
export I_MPI_OFI_LIBRARY=/opt/cray/libfabric/1.15.2.0/lib64/libfabric.so.1
export FI_PROVIDER_PATH=/opt/cray/libfabric/1.15.2.0/lib64

export EnableImplicitScaling=0
