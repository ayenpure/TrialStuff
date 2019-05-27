cmake_minimum_required(VERSION 3.9)                                             
set (CMAKE_CXX_STANDARD 11)                                                     
                                                                                
project(makedataset)                                                         
                                                                                
find_package(VTKm REQUIRED QUIET                                                
             OPTIONAL_COMPONENTS TBB Serial CUDA
            )

add_executable(makedataset DoubleGyrefield.cxx GenerateDataSet.cxx)

target_include_directories(makedataset PRIVATE ${VTKm_INCLUDE_DIRS})         
target_link_libraries(makedataset PRIVATE vtkm_cont vtkm_worklet) 
target_compile_options(makedataset PRIVATE ${VTKm_COMPILE_OPTIONS})              
