cmake_minimum_required(VERSION 3.9)                                             
set (CMAKE_CXX_STANDARD 11)                                                     
                                                                                
project(cellpointextract)                                                         
                                                                                
find_package(VTKm REQUIRED QUIET                                                
             OPTIONAL_COMPONENTS TBB Serial CUDA Rendering                      
            )                                                                   
                                                                                
add_executable(cellpointextract forSud.cxx)                       
target_include_directories(cellpointextract PRIVATE ${VTKm_INCLUDE_DIRS})         
target_link_libraries(cellpointextract ${VTKm_LIBRARIES})                         
target_compile_options(cellpointextract PRIVATE ${VTKm_COMPILE_OPTIONS})  
