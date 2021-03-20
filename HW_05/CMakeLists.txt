cmake_minimum_required(VERSION 3.10)
project(HW_05)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -g")
file(GLOB SOURCES
        *.hpp
        *.inl
        *.cpp
        )
add_executable(HW_05 main.cpp slab_allocator.cpp mmap_page_allocator.hpp mmap_page_allocator.inl page_allocator.hpp slab_allocator.hpp)