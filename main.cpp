#include <ishmem.h>
#include <mpi.h>
#include <sycl/sycl.hpp>

int main() {

#ifdef USE_MPI
  std::cout << "MPI is used\n";
  MPI_Init(nullptr, nullptr);
#endif

#ifdef USE_ISHMEM
  std::cout << "ISHMEM is used\n";
  ishmem_init();
#endif

#ifdef USE_MPI
  sycl::usm::alloc kind = sycl::usm::alloc::host; // problem is with all shared/device/host
  sycl::queue sycl_queue_;
  std::byte* mem = sycl::malloc<std::byte>(40, sycl_queue_, kind);
  MPI_Win win = MPI_WIN_NULL;
  MPI_Win_create(mem, 40, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &win);

  MPI_Win_free(&win);
  sycl::free(mem, sycl_queue_);
#endif

#ifdef USE_ISHMEM
  ishmem_finalize();
#endif

#ifdef USE_MPI
  MPI_Finalize();
#endif
}
