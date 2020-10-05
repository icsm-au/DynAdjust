# <dnaimportwrapper/...> build rules
project (dnaimportwrapper)

# build dnaimport library first
add_subdirectory(${CMAKE_SOURCE_DIR}/dynadjust/dnaimport ${CMAKE_BINARY_DIR}/dynadjust/dnaimport)

add_definitions (-DBUILD_IMPORT_EXE)

include_directories(${PROJECT_SOURCE_DIR})

add_executable (${PROJECT_NAME} 
                ${CMAKE_SOURCE_DIR}/include/io/dnaiobase.cpp
                ${CMAKE_SOURCE_DIR}/include/io/dnaiobms.cpp
                ${CMAKE_SOURCE_DIR}/include/io/dnaiobst.cpp
                ${CMAKE_SOURCE_DIR}/include/config/dnaprojectfile.cpp
                ${CMAKE_SOURCE_DIR}/include/parameters/dnaprojection.cpp
                ${CMAKE_SOURCE_DIR}/include/measurement_types/dnamsrtally.cpp
                ${CMAKE_SOURCE_DIR}/include/measurement_types/dnastntally.cpp
                ${CMAKE_SOURCE_DIR}/include/functions/dnastringfuncs.cpp
                dnaimportwrapper.cpp)

target_link_libraries (${PROJECT_NAME} dnaimport ${DNA_LIBRARIES})

set_target_properties (${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${DNA_PROGRAM_PREFIX}import")

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
