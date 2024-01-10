#include <ishmem.h>
#include <mpi.h>

int main() {
  
  MPI_Init(nullptr, nullptr);
  ishmem_init();

  ishmem_finalize();
  MPI_Finalize();
}
