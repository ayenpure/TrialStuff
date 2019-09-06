cmake_minimum_required(VERSION 3.9)                                             
set (CMAKE_CXX_STANDARD 11)                                                     
                                                                                
project(printdata)                                                         
                                                                                
find_package(VTKm REQUIRED QUIET                                                
             OPTIONAL_COMPONENTS TBB Serial CUDA
            )

add_executable(printdata ArrangeData.cxx)

target_include_directories(printdata PRIVATE ${VTKm_INCLUDE_DIRS})         
target_link_libraries(printdata PRIVATE vtkm_cont vtkm_worklet) 
target_compile_options(printdata PRIVATE ${VTKm_COMPILE_OPTIONS})              
