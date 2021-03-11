#include <algorithm>
#include <cmath>
#include "BuddyAllocator.hpp"

namespace hse::arch_os
{

  BuddyAllocator::BuddyAllocator(void *memoryStart, std::size_t memoryLength)
  {
    // Evaluate binary tree size and memory
    std::size_t fullTreeSize = calculateFullTreeSizeFromMemLen(memoryLength);
    std::size_t memorySizeForDescriptors = fullTreeSize * sizeof(Descriptor);
    std::size_t additionalMemory = memorySizeForDescriptors + memorySizeForLists;

    std::size_t treeMemSize = memoryLength - additionalMemory;
    std::size_t actualTreeSize = calculateFullTreeSizeFromMemLen(treeMemSize);
    localMaxOrder = std::log2(treeMemSize / PAGESIZE);

    char *memStartReinterpreted = reinterpret_cast<char *>(memoryStart);
    memTreeLimit = memStartReinterpreted + treeMemSize - PAGESIZE;
    // Allocate memory for descriptors and lists
    descriptors = new (memStartReinterpreted + memoryLength - memorySizeForDescriptors) Descriptor[fullTreeSize];
    lists = new (memStartReinterpreted + memoryLength - additionalMemory) std::size_t[MAX_ORDER];

    for (std::ptrdiff_t i = 0; i < MAX_ORDER; ++i)
      lists[i] = -1;

    // Build binary tree
    buildTree(1, 0, (actualTreeSize + 1) / 2 - 1, memStartReinterpreted, 0);

    // If tree is fully filled
    if (descriptors[0].isFree)
    {
      lists[localMaxOrder] = 0;
      isFullTree = true;
    }
  }

  void *BuddyAllocator::allocate(std::size_t order)
  {
    if (order > localMaxOrder)
      return nullptr;
    std::size_t tmpOrder = order;
    // Seek for big enough node
    while (tmpOrder <= localMaxOrder && lists[tmpOrder] == static_cast<std::size_t>(-1))
      tmpOrder++;
    // But not too big
    if (tmpOrder > localMaxOrder)
      return nullptr;

    // Split node
    std::size_t node = lists[tmpOrder];
    while (order != tmpOrder)
    {
      descriptors[node].isFree = false;
      removeFromList(node, tmpOrder);
      if (descriptors[2 * node + 2].isFree)
        addToList(2 * node + 2, tmpOrder - 1);

      node = 2 * node + 1;
      tmpOrder--;
    }

    descriptors[node].isFree = false;
    removeFromList(node, tmpOrder);

    return descriptors[node].memPtr;
  }

  void BuddyAllocator::deallocate(void *memory)
  {
    std::size_t node = 0;
    std::size_t currentOrder = localMaxOrder + !isFullTree;

    // Find branch
    while (memory != descriptors[node].memPtr)
    {
      if (memory < descriptors[2 * node + 2].memPtr)
        node = 2 * node + 1;
      else
        node = 2 * node + 2;
      currentOrder--;
    }

    // Go down along brunch and find allocation
    while (currentOrder > 0 && !descriptors[2 * node + 1].isFree)
    {
      node = 2 * node + 1;
      currentOrder--;
    }

    // Free and lift with merging nodes
    descriptors[node].isFree = true;
    while (node != 0)
    {
      std::size_t buddy = getBuddy(node);
      if (!descriptors[buddy].isFree)
        break;
      removeFromList(buddy, currentOrder);
      node = (node - 1) / 2;
      descriptors[node].isFree = true;
      currentOrder++;
    }
    addToList(node, currentOrder);
  }

  std::size_t BuddyAllocator::calculateFullTreeSizeFromMemLen(std::size_t memoryLength)
  {
    return std::pow(2,std::ceil(std::log2(std::min(memoryLength / PAGESIZE, maxNumberOfDescriptors))) + 1) - 1;
  }

    void BuddyAllocator::buildTree(std::size_t v, std::size_t l, std::size_t r, char *mem, std::size_t level)
    {
      // Building like segment tree
      if (l == r)
      {
        if (memTreeLimit < mem)
        {
          descriptors[v - 1].isFree = false;
          descriptors[v - 1].memPtr = nullptr;
        }
        else
          descriptors[v - 1].memPtr = mem;
        return;
      }
      std::size_t m = (l + r) / 2;

      buildTree(2 * v, l, m, mem, level + 1);
      buildTree(2 * v + 1, m + 1, r, mem + (m + 1 - l) * PAGESIZE, level + 1);
      descriptors[v - 1].memPtr = mem;
      descriptors[v - 1].isFree = descriptors[2 * v - 1].isFree && descriptors[2 * v].isFree;

      if (!descriptors[2 * v].isFree && descriptors[2 * v - 1].isFree)
        lists[localMaxOrder - level] = 2 * v - 1;
    }

  void BuddyAllocator::removeFromList(std::size_t node, std::size_t listId)
  {
    Descriptor &descr = descriptors[node];
    if (descr.prev != static_cast<std::size_t>(-1))
      descriptors[descr.prev].next = descr.next;
    if (descr.next != static_cast<std::size_t>(-1))
      descriptors[descr.next].prev = descr.prev;
    if (lists[listId] == node)
      lists[listId] = descr.next;
    descr.next = descr.prev = -1;
  }

  void BuddyAllocator::addToList(std::size_t node, std::size_t listId)
  {
    descriptors[node].next = lists[listId];
    if (lists[listId] != static_cast<std::size_t>(-1))
      descriptors[lists[listId]].prev = node;
    lists[listId] = node;
  }

  std::size_t BuddyAllocator::getBuddy(std::size_t node)
  {
    if (node % 2 == 1)
      return node + 1;
    return node - 1;
  }

}

