cmake_minimum_required(VERSION 3.15)
project(raytracer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenMP)

add_executable(raytracer src/main.cpp)
target_include_directories(raytracer PRIVATE src third_party)
if(OpenMP_CXX_FOUND)
  target_link_libraries(raytracer PRIVATE OpenMP::OpenMP_CXX)
  message(STATUS "OpenMP enabled")
endif()