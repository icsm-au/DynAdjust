cmake_minimum_required (VERSION 3.0)

project (dynadjust)

set (DYNADJUST_VERSION "1.0.3")

if (BUILD_TESTING)
    enable_testing()
endif (BUILD_TESTING)

message (STATUS " " )
message (STATUS "**********************************************" )
message (STATUS "Configuring DynAdjust ${DYNADJUST_VERSION} (${CMAKE_BUILD_TYPE}) build using cmake..." )
message (STATUS " " )

if(UNIX)
   set(DNA_PROGRAM_PREFIX "dna")
endif(UNIX)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR};${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 14)
add_definitions(-Wno-deprecated-declarations)

if (BUILD_TESTING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -coverage -lgcov")
endif (BUILD_TESTING)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl")
else()
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

message (STATUS "Looking for xerces-c..." )
find_package (XercesC COMPONENTS xerces-c REQUIRED)
if (NOT XERCESC_FOUND )
   set (DNA_BUILD_ERROR 1)
   set (XERCES_FIND_ERROR 1)
   message (SEND_ERROR "Cannot find XercesC library")
else ()
   message (STATUS "xerces-c seems to be installed correctly." )
endif ()
message (STATUS " " )

message (STATUS "Looking for xsd headers..." )
find_package (XSD REQUIRED)
if (NOT XSD_FOUND )
   set (DNA_BUILD_ERROR 1)
   set (XSD_FIND_ERROR 1)
   message (SEND_ERROR "Cannot find XSD library")
else ()
   message (STATUS "xsd headers seem to be installed correctly." )
endif ()
message (STATUS " " )

message (STATUS "Looking for boost..." )
set (Boost_USE_MULTITHREADED ON)
set (BOOST_MIN_VERSION "1.58.0")
find_package (Boost ${BOOST_MIN_VERSION} COMPONENTS system filesystem timer thread program_options REQUIRED)
if (NOT Boost_FOUND )
   set (DNA_BUILD_ERROR 1)
   set (BOOST_FIND_ERROR 1)
   message (SEND_ERROR "Cannot find Boost library components")
else ()
   message (STATUS "boost seems to be installed correctly.")
endif ()
message (STATUS " ")

message (STATUS "Looking for intel math kernel library (mkl)..." )
find_package (MKL COMPONENTS mkl_blas95_ilp64 mkl_intel_ilp64 mkl_intel_thread mkl_core REQUIRED)
if (NOT MKL_FOUND )
   set (DNA_BUILD_ERROR 1)
   set (MKL_FIND_ERROR 1)
   message (SEND_ERROR "Cannot find MKL library components")
else ()
    message (STATUS "MKL seems to be installed correctly.")
endif ()
message (STATUS " ")

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    find_library(IOMP5_LIB iomp5 PATHS /opt/intel/lib)
else()
    find_library(IOMP5_LIB iomp5 PATHS /opt/intel/lib/intel64)
endif()
if( NOT IOMP5_LIB )
   set(DNA_BUILD_ERROR 1)
   set (IOMP5_FIND_ERROR 1)
   message(SEND_ERROR "Cannot find iomp5 library")
endif()

set (DNA_LIBRARIES ${Boost_LIBRARIES} ${XERCESC_LIBRARY} ${MKL_LIBRARIES} ${IOMP5_LIB})

message (STATUS "DynAdjust library dependencies:")
message (STATUS ${DNA_LIBRARIES})
message (STATUS "")

include_directories( ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIRS} ${MKL_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIR} ${XSD_INCLUDE_DIR})

if (DNA_BUILD_ERROR)
    message (STATUS " " )
    message (STATUS "**********************************************" )
    message (STATUS "Cannot build DynaAjust ${DYNADJUST_VERSION}.  Missing components:")
if (XERCES_FIND_ERROR)
    message (STATUS "  xerces-c")
endif ()
if (XSD_FIND_ERROR)
    message (STATUS "  xsd headers")
endif ()
if (BOOST_FIND_ERROR)
    message (STATUS "  boost")
endif ()
if (MKL_FIND_ERROR)
    message (STATUS "  mkl")
endif ()
if (IOMP5_FIND_ERROR)
    message (STATUS "  iomp5")
endif ()
    message (STATUS " ")
    message (FATAL_ERROR "Build terminating.")
endif ()

message (STATUS "configuring import" )
add_subdirectory (dynadjust/dnaimportwrapper)

message (STATUS "configuring reftran" )
add_subdirectory (dynadjust/dnareftranwrapper)

message (STATUS "configuring segment" )
add_subdirectory (dynadjust/dnasegmentwrapper)

message (STATUS "configuring geoid" )
add_subdirectory (dynadjust/dnageoidwrapper)

message (STATUS "configuring adjust" )
add_subdirectory (dynadjust/dnaadjustwrapper)

message (STATUS "configuring plot" )
add_subdirectory (dynadjust/dnaplotwrapper)

message (STATUS "configuring dynadjust" )
add_subdirectory (dynadjust/dynadjust)

if (BUILD_TESTING)
    message (STATUS "  " )
    message (STATUS "configuring tests" )
    # test execution of dynadjust binaries
    # 1. gnss network (simultaneous)
    add_test (NAME import-gnss-network COMMAND $<TARGET_FILE:dnaimportwrapper> -n gnss ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.msr)
    add_test (NAME geoid-gnss-network COMMAND $<TARGET_FILE:dnageoidwrapper> gnss -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb --convert-stn-hts)
    add_test (NAME adjust-gnss-network COMMAND $<TARGET_FILE:dnaadjustwrapper> gnss --output-adj-msr)
    # 2. urban network (phased-sequential)
    add_test (NAME import-urban-network COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.msr)
    add_test (NAME geoid-urban-network COMMAND $<TARGET_FILE:dnageoidwrapper> urban -g ${CMAKE_SOURCE_DIR}/../sampleData/urban-network-geoid.gsb --convert-stn-hts)
    add_test (NAME segment-urban-network COMMAND $<TARGET_FILE:dnasegmentwrapper> urban --min 50 --max 150)
    add_test (NAME adjust-urban-network COMMAND $<TARGET_FILE:dnaadjustwrapper> urban --output-adj-msr --phased)
    # 3. urban network (transform to GDA2020, phased-concurrent)
    add_test (NAME import-urban-network-thread COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_mt ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.msr)
    add_test (NAME reftran-urban-network-thread COMMAND $<TARGET_FILE:dnareftranwrapper> urban_mt -r gda2020)
    add_test (NAME geoid-urban-network-thread COMMAND $<TARGET_FILE:dnageoidwrapper> urban_mt -g ${CMAKE_SOURCE_DIR}/../sampleData/urban-network-geoid.gsb --convert-stn-hts)
    add_test (NAME segment-urban-network-thread COMMAND $<TARGET_FILE:dnasegmentwrapper> urban_mt --min 50 --max 85)
    add_test (NAME adjust-urban-network-thread COMMAND $<TARGET_FILE:dnaadjustwrapper> urban_mt --multi --output-adj-msr)
    
    # bash command to check results
    add_test (NAME test-gnss-network COMMAND bash -c "diff <(tail -n +53 gnss.simult.adj) <(tail -n +53 ${CMAKE_SOURCE_DIR}/../sampleData/gnss.simult.adj.expected)")
    add_test (NAME test-urban-phased-network COMMAND bash -c "diff <(tail -n +62 urban.phased.adj) <(tail -n +62 ${CMAKE_SOURCE_DIR}/../sampleData/urban.phased.adj.expected)")
    add_test (NAME test-urban-thread-network COMMAND bash -c "diff <(tail -n +64 urban_mt.phased-mt.adj) <(tail -n +64 ${CMAKE_SOURCE_DIR}/../sampleData/urban_mt.phased-mt.adj.expected)")

    # set execution dependencies (the execution of tests must be sequential)
    set_tests_properties(
        import-gnss-network geoid-gnss-network adjust-gnss-network
        PROPERTIES RUN_SERIAL TRUE)
    set_tests_properties(
        import-urban-network geoid-urban-network segment-urban-network adjust-urban-network
        PROPERTIES RUN_SERIAL TRUE)
    set_tests_properties(
        import-urban-network-thread reftran-urban-network-thread geoid-urban-network-thread segment-urban-network-thread adjust-urban-network-thread
        PROPERTIES RUN_SERIAL TRUE)
    set_tests_properties(test-gnss-network PROPERTIES DEPENDS adjust-gnss-network)
    set_tests_properties(test-urban-phased-network PROPERTIES DEPENDS adjust-urban-network)
    set_tests_properties(test-urban-thread-network PROPERTIES DEPENDS adjust-urban-network-thread)
endif ()

message (STATUS " " )
message (STATUS "finished configuring DynAdjust ${DYNADJUST_VERSION} (${CMAKE_BUILD_TYPE})." )
message (STATUS "**********************************************" )
message (STATUS " " )
#message (STATUS "Now run make to build DynAdjust ${DYNADJUST_VERSION} (${CMAKE_BUILD_TYPE}) binaries" )
#message (STATUS " " )
