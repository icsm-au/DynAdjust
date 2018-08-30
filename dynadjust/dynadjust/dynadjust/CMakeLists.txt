# <dynadjust/...> build rules
project (dynadjust)

set (CMAKE_BUILD_TYPE "Release")

add_definitions (-DBUILD_DYNADJUST_EXE)

include_directories ("${CMAKE_SOURCE_DIR}/dynadjust/${PROJECT_NAME}" ${PROJECT_SOURCE_DIR})

add_executable (${PROJECT_NAME} 
                ${CMAKE_SOURCE_DIR}/include/config/dnaprojectfile.cpp
                ${CMAKE_SOURCE_DIR}/include/functions/dnaprocessfuncs.cpp
                ${CMAKE_SOURCE_DIR}/include/functions/dnastringfuncs.cpp
                dynadjust.cpp)

target_link_libraries (${PROJECT_NAME} ${DNA_LIBRARIES})

install (TARGETS ${PROJECT_NAME} DESTINATION bin)
