cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set (CMAKE_CXX_STANDARD 11)
project(exepasa CXX)

#Find the VTK-m package
find_package(VTKm REQUIRED QUIET)

add_executable(exepasa pasa.cxx)
target_link_libraries(exepasa PRIVATE vtkm_cont)

if(TARGET vtkm::tbb)
  add_executable(exepasaTBB pasaTBB.cxx)
  target_link_libraries(exepasaTBB PRIVATE vtkm_cont)
endif()

if(TARGET vtkm::cuda)
  add_executable(exepasaCUDA pasaCUDA.cu)
  target_link_libraries(exepasaCUDA PRIVATE vtkm_cont)
endif()
