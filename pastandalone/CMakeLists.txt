cmake_minimum_required(VERSION 3.9)                                             
set (CMAKE_CXX_STANDARD 11)                                                     
                                                                                
project(exepasa)                                                         
                                                                                
find_package(VTKm REQUIRED QUIET                                                
             OPTIONAL_COMPONENTS TBB Serial CUDA
            )                                                                   
                                                                                
add_executable(exepasa pasa.cxx)                       
target_include_directories(exepasa PRIVATE ${VTKm_INCLUDE_DIRS})         
target_link_libraries(exepasa ${VTKm_LIBRARIES})                         
target_compile_options(exepasa PRIVATE ${VTKm_COMPILE_OPTIONS})

if(VTKm_TBB_FOUND)
  add_executable(exepasaTBB pasaTBB.cxx)                       
  target_include_directories(exepasaTBB PRIVATE ${VTKm_INCLUDE_DIRS})         
  target_link_libraries(exepasaTBB ${VTKm_LIBRARIES})                         
  target_compile_options(exepasaTBB PRIVATE ${VTKm_COMPILE_OPTIONS})
endif()

if(VTKm_CUDA_FOUND)
  # Cuda compiles do not respect target_include_directories
  cuda_include_directories(${VTKm_INCLUDE_DIRS})
  cuda_add_executable(exepasaCUDA pasaCUDA.cu)
  target_include_directories(exepasaCUDA PRIVATE ${VTKm_INCLUDE_DIRS})         
  target_link_libraries(exepasaCUDA PRIVATE ${VTKm_LIBRARIES})                         
  target_compile_options(exepasaCUDA PRIVATE ${VTKm_COMPILE_OPTIONS})
endif()
