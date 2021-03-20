#include <cassert>
#include <cmath>

#include "slab_allocator.hpp"
#include "page_allocator.hpp"

namespace hse::arch_os
{
  SlabAllocator::SlabAllocator(std::size_t objectSize) : objectSize{objectSize}
  {
    assert(objectSize >= 0);
    if (objectSize >= BIG_SIZE && objectSize <= MAX_SLAB_SIZE)
    {
      assert(sizeof(SlabAllocator) <= PAGE_SIZE);
      void *mem = pageAllocator().allocate(0);
      assert(mem != nullptr);
      ctrlAllocator = new(mem) SlabAllocator(CTRL_SIZE);
    }
    orderOfNeededPages = std::ceil(std::log2(CAPACITY_FOR_BIG_OBJECTS * static_cast<double>(objectSize) / PAGE_SIZE));
  }

  SlabAllocator::~SlabAllocator()
  {
    while (slabs != nullptr)
    {
      SlabPageDescriptor *tmp = slabs->next;
      pageAllocator().deallocate(slabs->ptrPageAddress);
      slabs = tmp;
    }
    if (objectSize >= BIG_SIZE && objectSize <= MAX_SLAB_SIZE)
    {
      void *allocMem = &ctrlAllocator;
      ctrlAllocator->~SlabAllocator();
      pageAllocator().deallocate(allocMem);
    }
  }

  void * SlabAllocator::allocate()
  {
    if (objectSize > MAX_SLAB_SIZE)
      return pageAllocator().allocate(orderOfNeededPages);

    SlabPageDescriptor *cur = slabs;
    SlabPageDescriptor *prev = nullptr;

    while (cur != nullptr)
    {
      if (cur->freed != 0)
        return takeMemFromSlab(cur);

      prev = cur;
      cur = cur->next;
    }
    if (objectSize < BIG_SIZE)
    {
      if (prev == nullptr)
      {
        slabs = createNewSlabSmall();
        return takeMemFromSlab(slabs);
      }
      prev->next = createNewSlabSmall();
      return takeMemFromSlab(prev->next);
    }
    else
    {
      if (prev == nullptr)
      {
        slabs = createNewSlabBig();
        return takeMemFromSlab(slabs);
      }
      prev->next = createNewSlabBig();
      return takeMemFromSlab(prev->next);
    }

    return nullptr;
  }
  void SlabAllocator::deallocate(void * memory)
  {
    if (memory == nullptr)
      return;

    if (objectSize > MAX_SLAB_SIZE)
      pageAllocator().deallocate(memory);
    else
    {
      SlabPageDescriptor *pageDescriptor = &pageAllocator().pageDescriptor(
              (void *)((uintptr_t)memory  / PAGE_SIZE * PAGE_SIZE));
      pageDescriptor = pageDescriptor->mainDescriptor;
      SlabCtrlBase *ctrl = pageDescriptor->slab;

      while (ctrl->mem != memory && ctrl != nullptr)
        ctrl = ctrl->next;
      if (ctrl == nullptr)
        return;
      ctrl->isFree = true;
      pageDescriptor->freed += 1;
      if (pageDescriptor->freed == pageDescriptor->capacity)
      {
        if (pageDescriptor == slabs)
          slabs = slabs->next;
        else
        {
          SlabPageDescriptor *cur = slabs;
          while (cur->next != pageDescriptor)
            cur = cur->next;
          cur->next = pageDescriptor->next;
        }
        pageAllocator().deallocate(pageDescriptor->ptrPageAddress);
      }
    }
  }

  SlabPageDescriptor *SlabAllocator::createNewSlabSmall()
  {
    void *mem = pageAllocator().allocate(0);
    if (mem == nullptr)
      return nullptr;
    SlabPageDescriptor *pageDescriptor = &pageAllocator().pageDescriptor(mem);
    std::size_t capacity = (PAGE_SIZE) / (CTRL_SIZE + objectSize);

    pageDescriptor->ptrPageAddress = mem;
    pageDescriptor->capacity = capacity;
    pageDescriptor->freed = capacity;
    pageDescriptor->mainDescriptor = pageDescriptor;
    SlabCtrlBase *prev = pageDescriptor->slab = new(mem) SlabCtrlBase();

    prev->isFree = true;
    mem += CTRL_SIZE;
    prev->mem = mem;
    prev->isFree = true;
    mem += objectSize;

    for (std::size_t i = 1; i < capacity; ++i)
    {
      prev->next = new(mem) SlabCtrlBase();
      prev = prev->next;
      prev->isFree = true;
      mem += CTRL_SIZE;
      prev->mem = mem;
      mem += objectSize;
    }

    return pageDescriptor;
  }

  void *SlabAllocator::takeMemFromSlab(SlabPageDescriptor *slab)
  {
    if (slab == nullptr)
      return nullptr;
    SlabCtrlBase *descr = slab->slab;
    while (!descr->isFree)
      descr = descr->next;

    descr->isFree = false;
    slab->freed -= 1;

    return descr->mem;
  }

  SlabPageDescriptor *SlabAllocator::createNewSlabBig()
  {
    void *mem = pageAllocator().allocate(orderOfNeededPages);
    if (mem == nullptr)
      return nullptr;
    SlabPageDescriptor *pageDescriptor = &pageAllocator().pageDescriptor(mem);
    std::size_t capacity = CAPACITY_FOR_BIG_OBJECTS;

    pageDescriptor->ptrPageAddress = mem;
    pageDescriptor->capacity = capacity;
    pageDescriptor->freed = capacity;
    pageDescriptor->mainDescriptor = pageDescriptor;

    for (std::size_t i = 1; i < 1ull << orderOfNeededPages; ++i)
    {
      SlabPageDescriptor *slave = &pageAllocator().pageDescriptor(mem + i * PAGE_SIZE);
      slave->mainDescriptor = pageDescriptor;
    }
    SlabCtrlBase *prev = pageDescriptor->slab = new(ctrlAllocator->allocate()) SlabCtrlBase();

    if (prev == nullptr)
    {
      pageDescriptor->capacity = 0;
      return pageDescriptor;
    }

    prev->isFree = true;
    prev->mem = mem;
    mem += objectSize;

    for (std::size_t i = 1; i < capacity; ++i)
    {
      prev->next = new(ctrlAllocator->allocate()) SlabCtrlBase();
      if (prev->next == nullptr)
      {
        pageDescriptor->capacity = i;
        return pageDescriptor;
      }
      prev = prev->next;
      prev->isFree = true;
      prev->mem = mem;
      mem += objectSize;
    }

    return pageDescriptor;
  }

}