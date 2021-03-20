#pragma once

#include <optional>
#include <functional>
#include <map>

namespace hse::arch_os {
  template<typename PageDescriptor>
  class MmapPageAllocator {
  public:
    MmapPageAllocator(){
      //descriptors.
    };
    ~MmapPageAllocator() = default;

    void *allocate(std::size_t order);
    PageDescriptor &pageDescriptor(void *address);
    void deallocate(void *address);

  private:
    struct Descriptor {
      std::size_t length;
      PageDescriptor userDescriptor;
    };

    std::map<void *, Descriptor, std::less<>> descriptors;
  };
}

#include "mmap_page_allocator.inl"