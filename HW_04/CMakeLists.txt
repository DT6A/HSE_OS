cmake_minimum_required(VERSION 3.10)
project(HW_04)

add_compile_options(-Wall -Wextra -Werror)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -g")

set(CMAKE_CXX_STANDARD 17)

add_executable(HW_04 main.cpp BuddyAllocator.cpp)