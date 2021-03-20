#pragma once

#include <sys/user.h>

#include <cstddef>

namespace hse::arch_os {

  struct SlabCtrlBase {
    void *mem = nullptr;
    bool isFree = false;
    SlabCtrlBase *next = nullptr;
  };

  struct SlabPageDescriptor {
    SlabCtrlBase *slab = nullptr;
    SlabPageDescriptor *next = nullptr;
    void *ptrPageAddress = nullptr;
    SlabPageDescriptor *mainDescriptor = nullptr;
    std::size_t capacity = 0;
    std::size_t freed = 0;
  };

  class SlabAllocator {
  public:
    static constexpr std::size_t BIG_SIZE = PAGE_SIZE / 8;

    SlabAllocator(std::size_t objectSize);
    ~SlabAllocator();
    void * allocate();
    void deallocate(void * memory);

  private:
    static constexpr std::size_t MAX_SLAB_SIZE = 8192;
    static constexpr std::size_t CAPACITY_FOR_BIG_OBJECTS = 8;
    static constexpr std::size_t CTRL_SIZE = sizeof(SlabCtrlBase);
    static constexpr std::size_t PAGE_DESCR_SIZE = sizeof(SlabPageDescriptor);
    std::size_t objectSize;
    std::size_t orderOfNeededPages = 0;
    SlabPageDescriptor *slabs = nullptr;
    SlabAllocator *ctrlAllocator;

    SlabPageDescriptor * createNewSlabSmall();
    SlabPageDescriptor * createNewSlabBig();
    void *takeMemFromSlab(SlabPageDescriptor *slab);

  };
}