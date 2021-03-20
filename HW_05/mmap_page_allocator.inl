#include "mmap_page_allocator.hpp"

#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

namespace hse::arch_os {
  template<typename PageDescriptor>
  void * MmapPageAllocator<PageDescriptor>::allocate(std::size_t order) {
    std::size_t pageSize = sysconf(_SC_PAGESIZE);

    void * p = mmap(
            nullptr, pageSize << order,
            PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE,
            -1, 0
    );
    if (p == MAP_FAILED)
      return nullptr;
    for (std::size_t i = 0; i != 1ull << order; ++i)
      descriptors[(void*)((uintptr_t)p + pageSize * i)] = { 1ull << order, {}};
    return p;
  }

  template<typename PageDescriptor>
  PageDescriptor & MmapPageAllocator<PageDescriptor>::pageDescriptor(void * address) {
    return descriptors[address].userDescriptor;
  }

  template<typename PageDescriptor>
  void MmapPageAllocator<PageDescriptor>::deallocate(void * address) {
    std::size_t pageSize = sysconf(_SC_PAGESIZE);

    Descriptor pd = descriptors[address];
    munmap(address, pageSize * pd.length);
    for (std::size_t i = 0; i < pd.length; ++i)
      descriptors.erase((void*)((uintptr_t)(address) + pageSize * i));
  }
}