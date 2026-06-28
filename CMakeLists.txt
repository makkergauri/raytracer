cmake_minimum_required(VERSION 3.15)
project(raytracer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to an optimized build. CMake picks the right flags per compiler for
# Release (-O3 on GCC/Clang, /O2 on MSVC), so we don't hardcode them.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(raytracer
  src/main.cpp
)

# Headers live in src/; the stb single-header lives in third_party/.
target_include_directories(raytracer PRIVATE src third_party)

# OpenMP gives near-linear speedup across CPU cores. Optional but recommended.
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(raytracer PRIVATE OpenMP::OpenMP_CXX)
  message(STATUS "OpenMP enabled")
else()
  message(WARNING "OpenMP not found — rendering will be single-threaded")
endif()
