# <dnareftran/...> build rules
project (dnareftran)

add_definitions(-DBUILD_DNAREFTRAN_DLL -DBUILD_EXPORTDNATYPES -DMKL_ILP64 -openmp)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -Wall -O0")

# If testing, then include coverage and turn off optimisations for release builds
if (BUILD_TESTING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0")
else ()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif (BUILD_TESTING)

include_directories (${PROJECT_SOURCE_DIR})

add_library (${PROJECT_NAME} SHARED
			${CMAKE_SOURCE_DIR}/include/io/dnaiobase.cpp
			${CMAKE_SOURCE_DIR}/include/io/dnaiobms.cpp
			${CMAKE_SOURCE_DIR}/include/io/dnaiobst.cpp
			${CMAKE_SOURCE_DIR}/include/io/dnaiodna.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnaangle.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnacoordinate.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnadirection.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnadirectionset.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnadistance.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnagpsbaseline.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnagpspoint.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnaheight.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnaheightdifference.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnameasurement.cpp
			${CMAKE_SOURCE_DIR}/include/parameters/dnadatum.cpp
			${CMAKE_SOURCE_DIR}/include/parameters/dnaellipsoid.cpp
			${CMAKE_SOURCE_DIR}/include/parameters/dnaprojection.cpp
			${CMAKE_SOURCE_DIR}/include/measurement_types/dnastation.cpp
			${CMAKE_SOURCE_DIR}/include/math/dnamatrix_contiguous.cpp
			${CMAKE_SOURCE_DIR}/include/functions/dnastringfuncs.cpp
			dnareftran.cpp)

target_link_libraries (${PROJECT_NAME} ${DNA_LIBRARIES})

install (TARGETS ${PROJECT_NAME} DESTINATION lib)
