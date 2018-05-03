cmake_minimum_required(VERSION 3.9)                                             
set (CMAKE_CXX_STANDARD 11)                                                     
                                                                                
project(makearrayhandle)                                                         
                                                                                
find_package(VTKm REQUIRED QUIET                                                
             OPTIONAL_COMPONENTS TBB Serial CUDA Rendering                      
            )                                                                   
                                                                                
add_executable(makearrayhandle arrayhandle.cxx)                       
target_include_directories(makearrayhandle PRIVATE ${VTKm_INCLUDE_DIRS})         
target_link_libraries(makearrayhandle ${VTKm_LIBRARIES})                         
target_compile_options(makearrayhandle PRIVATE ${VTKm_COMPILE_OPTIONS})  
