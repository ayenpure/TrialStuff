cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(time CXX)

#Find the VTK-m package
find_package(VTKm REQUIRED QUIET)

add_executable(trial GridLocator.cxx)
target_link_libraries(trial PRIVATE vtkm_cont)
