# <dnageoid/...> build rules
project (dnageoid)

add_definitions(-DBUILD_DNAGEOID_DLL -DBUILD_EXPORTDNATYPES)

include_directories (${PROJECT_SOURCE_DIR})

add_library (${PROJECT_NAME} SHARED
             ${CMAKE_SOURCE_DIR}/include/io/dnaiobase.cpp
             ${CMAKE_SOURCE_DIR}/include/io/dnaiobst.cpp
             ${CMAKE_SOURCE_DIR}/include/measurement_types/dnastation.cpp
             ${CMAKE_SOURCE_DIR}/include/functions/dnastringfuncs.cpp
             dnageoid.cpp)

target_link_libraries (${PROJECT_NAME} ${DNA_LIBRARIES})

install (TARGETS ${PROJECT_NAME} DESTINATION lib)
