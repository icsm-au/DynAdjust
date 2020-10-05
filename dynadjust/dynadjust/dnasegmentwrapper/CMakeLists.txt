# <dnasegmentwrapper/...> build rules
project (dnasegmentwrapper)

# build dnasegment library first
add_subdirectory (${CMAKE_SOURCE_DIR}/dynadjust/dnasegment ${CMAKE_BINARY_DIR}/dynadjust/dnasegment)

add_definitions (-DBUILD_SEGMENT_EXE)

include_directories(${PROJECT_SOURCE_DIR})

add_executable (${PROJECT_NAME} 
                ${CMAKE_SOURCE_DIR}/include/config/dnaprojectfile.cpp
                ${CMAKE_SOURCE_DIR}/include/functions/dnastringfuncs.cpp
                dnasegmentwrapper.cpp)

target_link_libraries (${PROJECT_NAME} dnasegment ${DNA_LIBRARIES})

set_target_properties (${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${DNA_PROGRAM_PREFIX}segment")

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
