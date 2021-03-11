#include <iostream>
#include <sys/mman.h>
#include <cassert>
#include <vector>

#include "BuddyAllocator.hpp"

void * const regionStart = reinterpret_cast<void *>(0x0000'0800'0000'0000ull);
void * const allocated = mmap(
        regionStart, hse::arch_os::BuddyAllocator::PAGESIZE * 1000,
        PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
        -1, 0
);

void testAllocationsAndCreations()
{
  { hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 2);
    assert(allocator.allocate(1) == nullptr);
    assert(allocator.allocate(0) != nullptr);
    assert(allocator.allocate(0) == nullptr);}

  {hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 3);
  assert(allocator.allocate(1) != nullptr);
  assert(allocator.allocate(0) == nullptr);}

  {hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 3);
  assert(allocator.allocate(0) != nullptr);
  assert(allocator.allocate(1) == nullptr);
  assert(allocator.allocate(0) != nullptr);
  assert(allocator.allocate(0) == nullptr);}

  {hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 4);
  assert(allocator.allocate(2) == nullptr);
  assert(allocator.allocate(0) != nullptr);
  assert(allocator.allocate(0) != nullptr);
  assert(allocator.allocate(0) != nullptr);
  assert(allocator.allocate(1) == nullptr);
  assert(allocator.allocate(0) == nullptr);
  assert(allocator.allocate(0) == nullptr);}

  {hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 4);
  assert(allocator.allocate(1) != nullptr);
  assert(allocator.allocate(0) != nullptr);
  assert(allocator.allocate(0) == nullptr);
  assert(allocator.allocate(1) == nullptr);}

  {hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 4);
  assert(allocator.allocate(0) != nullptr);
  assert(allocator.allocate(1) != nullptr);
  assert(allocator.allocate(0) == nullptr);
  assert(allocator.allocate(1) == nullptr);}

  {hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 8);
  assert(allocator.allocate(3) == nullptr);
  assert(allocator.allocate(2) != nullptr);
  assert(allocator.allocate(2) == nullptr);
  assert(allocator.allocate(1) != nullptr);
  assert(allocator.allocate(0) != nullptr);
  assert(allocator.allocate(0) == nullptr);
  assert(allocator.allocate(1) == nullptr);}
}

void testFull()
{
  {
    hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 2);
    void *alloc1 = allocator.allocate(0);
    allocator.deallocate(alloc1);
    void *alloc2 = allocator.allocate(0);
    assert(alloc1 == alloc2);
  }
  {
    hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 3);
    void *alloc1 = allocator.allocate(0);
    void *alloc2 = allocator.allocate(0);
    assert(allocator.allocate(1) == nullptr);
    assert(allocator.allocate(0) == nullptr);
    allocator.deallocate(alloc1);
    assert(allocator.allocate(1) == nullptr);
    allocator.deallocate(alloc2);
    alloc2 = allocator.allocate(1);
    assert(alloc1 == alloc2);
    assert(allocator.allocate(0) == nullptr);
    allocator.deallocate(alloc2);
    alloc2 = allocator.allocate(0);
    assert(alloc1 == alloc2);
    alloc1 = allocator.allocate(0);
    assert(alloc1 != alloc2);
  }

  {
    hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 4);
    void *alloc3 = allocator.allocate(0);
    void *alloc1 = allocator.allocate(0);
    void *alloc2 = allocator.allocate(0);
    assert(allocator.allocate(1) == nullptr);
    assert(allocator.allocate(0) == nullptr);
    allocator.deallocate(alloc1);
    assert(allocator.allocate(1) == nullptr);
    allocator.deallocate(alloc2);
    alloc2 = allocator.allocate(1);
    assert(alloc1 == alloc2);
    allocator.deallocate(alloc3);
    alloc1 = allocator.allocate(0);
    assert(alloc1 == alloc3);
    allocator.deallocate(alloc1);
    allocator.deallocate(alloc2);
    alloc1 = allocator.allocate(1);
    assert(alloc1 != nullptr);
    alloc2 = allocator.allocate(0);
    assert(alloc2 == alloc3);
    allocator.deallocate(alloc1);
    alloc3 = allocator.allocate(0);
    allocator.deallocate(alloc2);
    assert(allocator.allocate(1) == nullptr);
    allocator.deallocate(alloc3);
  }

  {
    hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 9);
    void *alloc1 = allocator.allocate(3);
    assert(allocator.allocate(0) == nullptr);
    allocator.deallocate(alloc1);
    alloc1 = allocator.allocate(1);
    void *alloc2 = allocator.allocate(1);
    void *alloc3 = allocator.allocate(1);
    void *alloc4 = allocator.allocate(1);
    assert(alloc1 != nullptr);
    assert(alloc2 != nullptr);
    assert(alloc3 != nullptr);
    assert(alloc4 != nullptr);
    assert(alloc1 != alloc2);
    assert(alloc1 != alloc3);
    assert(alloc1 != alloc4);
    assert(alloc2 != alloc3);
    assert(alloc2 != alloc4);
    assert(alloc1 != alloc3);
    assert(alloc3 != alloc4);

    allocator.deallocate(alloc1);
    allocator.deallocate(alloc4);
    assert(allocator.allocate(2) == nullptr);
    allocator.deallocate(alloc2);
    alloc2 = allocator.allocate(2);
    assert(alloc1 == alloc2);
    assert(allocator.allocate(2) == nullptr);
    allocator.deallocate(alloc3);
    allocator.deallocate(alloc2);

    alloc2 = allocator.allocate(3);
    assert(alloc1 == alloc2);
    assert(allocator.allocate(0) == nullptr);
    allocator.deallocate(alloc2);

    for (int i = 0; i < 8; ++i)
      assert(allocator.allocate(0) != nullptr);
    assert(allocator.allocate(0) == nullptr);
  }
}

void testThousand()
{
  hse::arch_os::BuddyAllocator allocator(allocated, hse::arch_os::BuddyAllocator::PAGESIZE * 1000);
  std::vector<void *> ptrs;

  for (int stage = 0; stage < 5; stage++)
  {
    for (int i = 0; i < 950; i++)
    {
      void *ptr = allocator.allocate(0);
      assert(ptr != nullptr);
      ptrs.push_back(ptr);
    }

    for (auto ptr : ptrs)
      allocator.deallocate(ptr);
    ptrs.clear();

    for (int i = 0; i < 425; i++)
    {
      void *ptr = allocator.allocate(1);
      assert(ptr != nullptr);
      ptrs.push_back(ptr);
    }
    for (auto ptr : ptrs)
      allocator.deallocate(ptr);
    ptrs.clear();

    for (int i = 0; i < 237; i++)
    {
      void *ptr = allocator.allocate(2);
      assert(ptr != nullptr);
      ptrs.push_back(ptr);
    }
    for (auto ptr : ptrs)
      allocator.deallocate(ptr);
    ptrs.clear();

    for (int i = 0; i < 7; ++i)
    {
      void *ptr = allocator.allocate(7);
      assert(ptr != nullptr);
      ptrs.push_back(ptr);
    }
    for (int i = 0; i < 6; ++i)
    {
      void *ptr = allocator.allocate(3);
      assert(ptr != nullptr);
      ptrs.push_back(ptr);
    }
    for (auto ptr : ptrs)
      allocator.deallocate(ptr);
    ptrs.clear();

    for (int i = 9; i >= 7; i--)
    {
      void *ptr = allocator.allocate(i);
      assert(ptr != nullptr);
      ptrs.push_back(ptr);
    }
    for (int i = 0; i < 54; ++i)
    {
      void *ptr = allocator.allocate(0);
      assert(ptr != nullptr);
      ptrs.push_back(ptr);
    }

    for (auto ptr : ptrs)
      allocator.deallocate(ptr);
    ptrs.clear();
  }
}

int main()
{
  testAllocationsAndCreations();
  testFull();
  testThousand();

  std::cout << "Tests passed" << std::endl;
  return 0;
}
