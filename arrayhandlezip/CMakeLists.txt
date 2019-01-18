#Find the VTK-m package
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(ArrayHandleZip CXX)

#Find the VTK-m package
find_package(VTKm REQUIRED QUIET)

add_executable(exeserial ArrayHandleZip.cxx)
target_link_libraries(exeserial PRIVATE vtkm_cont)

if(TARGET vtkm::tbb)
  add_executable(exetbb ArrayHandleZipTBB.cxx)
  target_link_libraries(exetbb PRIVATE vtkm_cont)
endif()

if(TARGET vtkm::cuda)
  add_executable(execuda ArrayHandleZip.cu)
  target_link_libraries(execuda PRIVATE vtkm_cont)
endif()
