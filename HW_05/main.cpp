/*
 * Author: @kkarnauk 
 */

#include <algorithm>
#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include <set>
#include <string>
#include <random>

#include "slab_allocator.hpp"
#include "page_allocator.hpp"

using namespace hse::arch_os;

enum class Color {
  Blue,
  Green
};

template<Color T>
class colorText;

std::string getPrefix(const colorText<Color::Green> &) {
  return "\033[1;32m";
}

std::string getPrefix(const colorText<Color::Blue> &) {
  return "\033[1;34m";
}

template<Color T>
class colorText {
public:
  explicit colorText(const std::string &text_) :
          text{text_} {}

  friend std::ostream &operator<<(std::ostream &out, const colorText &color) {
    out << getPrefix(color) << color.text << "\033[0m";
    return out;
  }

private:
  const std::string &text;
};


std::mt19937 rnd(228);

std::vector<std::size_t> randomPermutation(std::size_t len) {
  std::vector<std::size_t> perm(len);
  for (std::size_t i = 0; i < len; i++) {
    perm[i] = i;
  }
  std::shuffle(perm.begin(), perm.end(), rnd);

  return perm;
}

char *tocharp(void *memory) {
  return reinterpret_cast<char *>(memory);
}

std::vector<std::uint8_t> corruptMemory(void *memory, std::size_t len) {
  char *memoryChar = tocharp(memory);
  for (std::size_t i = 0; i < len; i++) {
    memoryChar[i] = rnd() & 255;
  }

  std::vector<std::uint8_t> bytes(len);
  std::memcpy(bytes.data(), memoryChar, len);

  return bytes;
}

void testNotCorruptedMemory(void *memory, const std::vector<uint8_t> &values) {
  assert(std::memcmp(memory, values.data(), values.size()) == 0);
}

void testAllocDealloc(std::size_t totalSize, std::size_t minObjectSize, std::size_t maxObjectSize) {
  for (std::size_t i = minObjectSize; i <= maxObjectSize; i++) {
    const std::size_t count = totalSize / i;
    {
      SlabAllocator alloc(i);
      for (std::size_t j = 0; j < count; j++) {
        assert(alloc.allocate() != nullptr);
      }
    }
    {
      SlabAllocator alloc(i);
      std::vector<void *> mems(count);
      for (std::size_t j = 0; j < count; j++) {
        mems[j] = alloc.allocate();
        assert(mems[j] != nullptr);
      }

      for (std::size_t j : randomPermutation(count)) {
        alloc.deallocate(mems[j]);
      }

    }
  }
}

void testMemory(std::size_t totalSize, std::size_t minObjectSize, std::size_t maxObjectSize) {
  for (std::size_t i = minObjectSize; i <= maxObjectSize; i++) {
    const std::size_t count = totalSize / i;
    {
      SlabAllocator alloc(i);
      std::vector<void *> mems(count);
      std::vector<std::vector<std::uint8_t>> memValues(count);
      for (std::size_t j = 0; j < count; j++) {
        mems[j] = alloc.allocate();
        memValues[j] = corruptMemory(mems[j], i);
      }

      for (std::size_t j = 0; j < count; j++) {
        testNotCorruptedMemory(mems[j], memValues[j]);
      }


      for (std::size_t j : randomPermutation(count)) {
        testNotCorruptedMemory(mems[j], memValues[j]);
        alloc.deallocate(mems[j]);
      }

    }
  }
}

void stressTest(std::size_t objectSize, std::size_t iterations) {
  constexpr std::size_t RANDOM_CHECKS_COUNT = 10;
  {
    SlabAllocator alloc(objectSize);

    std::vector<void *> mems;
    std::vector<std::vector<uint8_t>> memValues;
    std::set<std::size_t> indices;

    auto getRandomIndex = [&]() {
      return *indices.lower_bound(abs(int(rnd())) % (*indices.rbegin() + 1));
    };

    for (std::size_t iter = 0; iter < iterations; iter++) {
      int command = rnd();
      if (command % 3 != 0) { // allocation
        mems.push_back(alloc.allocate());
        memValues.push_back(corruptMemory(mems.back(), objectSize));
        indices.insert(mems.size() - 1);
      } else {
        if (indices.empty()) {
          iterations++;
          continue;
        }

        std::size_t index = getRandomIndex();
        indices.erase(index);
        testNotCorruptedMemory(mems[index], memValues[index]);
        alloc.deallocate(mems[index]);
      }

      if (indices.empty()) {
        continue;
      }

      for (std::size_t i = 0; i < RANDOM_CHECKS_COUNT; i++) {
        std::size_t index = getRandomIndex();
        testNotCorruptedMemory(mems[index], memValues[index]);
      }
    }
  }

  std::cout << "Stress test. Object size = "
            << colorText<Color::Blue>(std::to_string(objectSize)) << ". "
            << colorText<Color::Green>("Completed.") << std::endl;
}

void testSmallAllocDealloc() {
  testAllocDealloc(SlabAllocator::BIG_SIZE << 7, 1, SlabAllocator::BIG_SIZE - 1);

  std::cout << "Test: small sizes. What: allocate/deallocate. "
            << colorText<Color::Green>("Completed.") << std::endl;
}

void testSmallMemory() {
  testMemory(SlabAllocator::BIG_SIZE << 6, 1, SlabAllocator::BIG_SIZE - 1);

  std::cout << "Test: small sizes. What: memory not corrupted. "
            << colorText<Color::Green>("Completed.") << std::endl;
}

void testSmallSizes() {
  testSmallAllocDealloc();
  testSmallMemory();

  for (std::size_t i = 1; i < SlabAllocator::BIG_SIZE; i *= (i < 10 ? 2 : 1.1)) {
    stressTest(i, 5000);
  }
}

constexpr std::size_t MAX_OBJECT_SIZE = PAGE_SIZE * 2;

void testBigAllocDealloc() {
  testAllocDealloc(SlabAllocator::BIG_SIZE << 8, SlabAllocator::BIG_SIZE, MAX_OBJECT_SIZE);

  std::cout << "Test: big sizes. What: allocate/deallocate. "
            << colorText<Color::Green>("Completed.") << std::endl;
}

void testBigMemory() {
  testMemory(SlabAllocator::BIG_SIZE << 7, SlabAllocator::BIG_SIZE, MAX_OBJECT_SIZE);

  std::cout << "Test: big sizes. What: memory not corrupted. "
            << colorText<Color::Green>("Completed.") << std::endl;
}

void testBigSizes() {
  testBigAllocDealloc();
  testBigMemory();

  for (std::size_t i = SlabAllocator::BIG_SIZE; i <= MAX_OBJECT_SIZE; i *= 1.07) {
    stressTest(i, 5000);
  }
}

int main() {
  testSmallSizes();
  testBigSizes();
}