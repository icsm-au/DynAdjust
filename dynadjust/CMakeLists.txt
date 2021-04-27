cmake_minimum_required (VERSION 3.0)

project (dynadjust)

set (DYNADJUST_VERSION "1.1.0")

if (BUILD_TESTING)
    enable_testing()
endif (BUILD_TESTING)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#set(CMAKE_VERBOSE_MAKEFILE ON)

message (" ")
message ("===========================================================================")
message ("Configuring DynAdjust ${DYNADJUST_VERSION} (${CMAKE_BUILD_TYPE}) build using cmake...")
message (" ")

if(UNIX)
   set(DNA_PROGRAM_PREFIX "dna")
endif(UNIX)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR};${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 14)
add_definitions(-Wno-deprecated-declarations)

# default CXX flags
if (BUILD_TESTING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")    # turn off warnings for unit testing and code coverage
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif (BUILD_TESTING)

# Debug CXX flags (GCC 4.8 introduced -Og for superior debugging experience. For older versions, use -O0)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -Og")

# If testing, then include coverage and turn off optimisations for release builds
if (BUILD_TESTING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif (BUILD_TESTING)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl")
else()
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

message (STATUS "Looking for xerces-c...")
find_package (XercesC COMPONENTS xerces-c REQUIRED)
if (NOT XERCESC_FOUND )
   set (DNA_BUILD_ERROR 1)
   set (XERCES_FIND_ERROR 1)
   message (SEND_ERROR "Cannot find XercesC library")
else ()
   message (STATUS "xerces-c seems to be installed correctly.")
endif ()
message (" ")

message (STATUS "Looking for xsd headers...")
find_package (XSD REQUIRED)
if (NOT XSD_FOUND )
   set (DNA_BUILD_ERROR 1)
   set (XSD_FIND_ERROR 1)
   message (SEND_ERROR "Cannot find XSD library")
else ()
   message (STATUS "xsd headers seem to be installed correctly.")
endif ()
message (" ")

message (STATUS "Looking for boost...")
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
message (" ")

message (STATUS "Looking for intel math kernel library (mkl)...")
find_package (MKL COMPONENTS mkl_blas95_ilp64 mkl_intel_ilp64 mkl_intel_thread mkl_core REQUIRED)
if (NOT MKL_FOUND )
   set (DNA_BUILD_ERROR 1)
   set (MKL_FIND_ERROR 1)
   message (SEND_ERROR "Cannot find MKL library components")
else ()
    message (STATUS "MKL seems to be installed correctly.")
endif ()
message (" ")

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
message (" ")

include_directories( ${CMAKE_SOURCE_DIR} ${Boost_INCLUDE_DIRS} ${MKL_INCLUDE_DIRS} ${XERCESC_INCLUDE_DIR} ${XSD_INCLUDE_DIR})

if (DNA_BUILD_ERROR)
    message (" ")
    message (STATUS "**********************************************")
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
    message (" ")
    message (FATAL_ERROR "Build terminating.")
endif ()

message (STATUS "Configuring import")
add_subdirectory (dynadjust/dnaimportwrapper)

message (STATUS "Configuring reftran")
add_subdirectory (dynadjust/dnareftranwrapper)

message (STATUS "Configuring segment")
add_subdirectory (dynadjust/dnasegmentwrapper)

message (STATUS "Configuring geoid")
add_subdirectory (dynadjust/dnageoidwrapper)

message (STATUS "Configuring adjust")
add_subdirectory (dynadjust/dnaadjustwrapper)

message (STATUS "Configuring plot")
add_subdirectory (dynadjust/dnaplotwrapper)

message (STATUS "Configuring dynadjust")
add_subdirectory (dynadjust/dynadjust)

if (BUILD_TESTING)
    message ("  ")
    message (STATUS "Configuring tests")
    
    # test execution of dynadjust binaries
    # 1. gnss network (simultaneous, phased-block 1, phased, staged)
    add_test (NAME import-gnss-network COMMAND $<TARGET_FILE:dnaimportwrapper> -n gnss ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.msr --export-dna --export-xml --export-asl --export-aml --export-map --output-msr-to-stn --test-integrity -r GDA94) 
    add_test (NAME geoid-gnss-network COMMAND $<TARGET_FILE:dnageoidwrapper> gnss -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb --convert-stn-hts --export-dna-geo)
    add_test (NAME reftran-gnss-network COMMAND $<TARGET_FILE:dnareftranwrapper> gnss -r gda2020 --export-dna --export-xml)
    add_test (NAME adjust-gnss-network COMMAND $<TARGET_FILE:dnaadjustwrapper> gnss --output-adj-msr --export-dna-msr --export-xml-msr)
    file (COPY ${CMAKE_SOURCE_DIR}/../sampleData/gnss_b1.net DESTINATION ./)
    add_test (NAME import-gnss-network-similar COMMAND $<TARGET_FILE:dnaimportwrapper> -n gnss_similar ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.msr -r itrf2008 --override-input-ref-frame --search-similar-gnss-msr --quiet) 
    add_test (NAME import-gnss-network-exclude COMMAND $<TARGET_FILE:dnaimportwrapper> -n gnss_excl ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.msr --exclude-stns-assoc-msrs "BEEC,261000380,356000780,349800490" --split) 
    add_test (NAME import-gnss-network-scaled COMMAND $<TARGET_FILE:dnaimportwrapper> -n gnss_scaled ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.msr --baseline-scalar-file ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.scalars -r GDA94) 
    add_test (NAME import-gnss-network-block1 COMMAND $<TARGET_FILE:dnaimportwrapper> -n gnss_b1 ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.msr --export-dna --export-xml --single-xml-file --export-asl --export-aml --export-map --output-msr-to-stn --test-integrity -r GDA94) 
    add_test (NAME segment-gnss-network-block1-a COMMAND $<TARGET_FILE:dnasegmentwrapper> gnss_b1 --min 90 --max 90 --starting-stns 220700210)
    add_test (NAME segment-gnss-network-block1-b COMMAND $<TARGET_FILE:dnasegmentwrapper> gnss_b1 --min 110 --max 110 --net-file)
    add_test (NAME segment-gnss-network-block1-c COMMAND $<TARGET_FILE:dnasegmentwrapper> gnss_b1 --min 80 --max 200 --net-file --starting-stns 211300940 --verbose 3)
    add_test (NAME adjust-gnss-network-block1 COMMAND $<TARGET_FILE:dnaadjustwrapper> gnss_b1 --block1-phased --output-adj-msr --output-adj-gnss-units 1)
    add_test (NAME adjust-gnss-network-phased COMMAND $<TARGET_FILE:dnaadjustwrapper> gnss_b1 --phased --output-adj-msr --output-adj-gnss-units 2)
    add_test (NAME adjust-gnss-network-verb COMMAND $<TARGET_FILE:dnaadjustwrapper> gnss_b1 --verbose 5)
    add_test (NAME adjust-gnss-network-staged-a COMMAND $<TARGET_FILE:dnaadjustwrapper> gnss_b1 --staged-adjustment --create-stage-files --output-adj-msr --output-adj-gnss-units 2 --sort-adj-msr-field 4)
    add_test (NAME adjust-gnss-network-staged-b COMMAND $<TARGET_FILE:dnaadjustwrapper> gnss_b1 --staged-adjustment --output-adj-msr --output-adj-gnss-units 3 --output-stn-blocks --output-msr-blocks --sort-adj-msr-field 5)
    
    # 2. urban network (phased-sequential)
    add_test (NAME import-urban-network COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.msr)
    add_test (NAME geoid-urban-network COMMAND $<TARGET_FILE:dnageoidwrapper> urban -g ${CMAKE_SOURCE_DIR}/../sampleData/urban-network-geoid.gsb --convert-stn-hts --export-dna-geo)
    add_test (NAME segment-urban-network COMMAND $<TARGET_FILE:dnasegmentwrapper> urban --min 50 --max 150 --test-integrity)
    add_test (NAME adjust-urban-network-verbose COMMAND $<TARGET_FILE:dnaadjustwrapper> urban --verbose 3)
    add_test (NAME adjust-urban-network COMMAND $<TARGET_FILE:dnaadjustwrapper> urban --output-adj-msr --phased --stn-corrections --export-sinex-file --export-xml-stn-file --export-dna-stn-file --output-pos-uncertainty --export-dna-msr --export-xml-msr)
    add_test (NAME plot-urban-network COMMAND $<TARGET_FILE:dnaplotwrapper> urban --supress-pdf --phased --label-sta --correction-arrows --label-corr --compute-corrections --scale-arrows 10.5 --error-ellipse --positional-uncertainty --scale-ellipse-c 10.5)
    add_test (NAME plot-urban-seg-stn COMMAND $<TARGET_FILE:dnaplotwrapper> urban --supress-pdf --graph-stn)
    add_test (NAME plot-urban-seg-msr COMMAND $<TARGET_FILE:dnaplotwrapper> urban --supress-pdf --graph-msr)
    
    # 3. urban network (transform to GDA2020, phased-concurrent)
    add_test (NAME import-urban-network-thread COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_mt ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.msr)
    add_test (NAME reftran-urban-network-thread COMMAND $<TARGET_FILE:dnareftranwrapper> urban_mt -r gda2020)
    add_test (NAME geoid-urban-network-thread COMMAND $<TARGET_FILE:dnageoidwrapper> urban_mt -g ${CMAKE_SOURCE_DIR}/../sampleData/urban-network-geoid.gsb --convert-stn-hts)
    add_test (NAME segment-urban-network-thread COMMAND $<TARGET_FILE:dnasegmentwrapper> urban_mt --min 50 --max 85 --verb 3)
    add_test (NAME adjust-urban-network-thread COMMAND $<TARGET_FILE:dnaadjustwrapper> urban_mt --output-adj-msr --multi  --free-stn-sd 4.0 --fixed-stn-sd 0.000001 --max-iterations 20 --output-tstat-adj-msr --sort-adj-msr-field 2 --sort-stn-orig-order --stn-coord-types PLHhENz --angular-stn-type 1 --angular-msr-type 1 --precision-stn-linear 3 --precision-msr-linear 3 --precision-stn-angular 4 --precision-msr-angular 4 --output-pos-uncertainty --output-all-covariances --output-corrections-file)
    add_test (NAME plot-urban-network-thread COMMAND $<TARGET_FILE:dnaplotwrapper> urban_mt --supress-pdf --phased --label-sta --label-font 16 --msr-line-w 0.5 --map-proj 3)

    # 4. urban network (phased-staged)
    add_test (NAME import-urban-network-stage COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_st ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.msr)
    add_test (NAME geoid-urban-network-stage COMMAND $<TARGET_FILE:dnageoidwrapper> urban_st -g ${CMAKE_SOURCE_DIR}/../sampleData/urban-network-geoid.gsb --convert-stn-hts --export-dna-geo)
    add_test (NAME segment-urban-network-stage COMMAND $<TARGET_FILE:dnasegmentwrapper> urban_st --min 90 --max 90)
    add_test (NAME adjust-urban-network-stage COMMAND $<TARGET_FILE:dnaadjustwrapper> urban_st --phased --staged-adjustment --create-stage-files --output-adj-msr --export-sinex-file --output-pos-uncertainty --export-xml-stn-file --export-xml-msr-file --export-dna-stn-file --export-dna-msr)
    
    # 5. dynadjust project mode
    file (COPY ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.stn DESTINATION ./)
    file (COPY ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.msr DESTINATION ./)
    add_test (NAME dynadjust-proj COMMAND $<TARGET_FILE:dynadjust> -p ${CMAKE_SOURCE_DIR}/../sampleData/sample.dnaproj --import --geoid --reftran --segment --adjust)

    # 6. urban data manipulation
    file (COPY ${CMAKE_SOURCE_DIR}/../sampleData/DynaML.xsd DESTINATION ./)
    add_test (NAME imp-urb00-dna-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_dna ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/urban-network.msr --export-xml --export-asl --export-aml --export-map --output-msr-to-stn)
    add_test (NAME imp-urb01-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_xml ./urban_dnastn.xml ./urban_dnamsr.xml --include-msr-types gxyemsabdpqr --exclude-msr-types vz)
    add_test (NAME imp-urb02-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_xml ./urban_dnastn.xml ./urban_dnamsr.xml --search-nearby-stn --nearby-stn-buffer 0.4)
    add_test (NAME imp-urb03-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_xml ./urban_dnastn.xml ./urban_dnamsr.xml --search-similar-msr --ignore-similar-msr --flag-unused-stations)
    add_test (NAME imp-urb04-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_xml ./urban_dnastn.xml ./urban_dnamsr.xml --v-scale 2.5 --p-scale 5.0 --l-scale 5.0 --h-scale 10.0)
    add_test (NAME imp-urb05-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_xml ./urban_dnastn.xml ./urban_dnamsr.xml --include-stns-assoc-msrs "4000,1,1004,1042,1046,2,2215,4000,5,9004,1002,1003,1034,1039,1050,13,2101,2102,2105,2106,2109,2118,2119,2122,2123,2124,2125,2206,2214")
    add_test (NAME imp-urb06-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_xml ./urban_dnastn.xml ./urban_dnamsr.xml --export-dna --export-asl --export-aml --export-map --output-msr-to-stn)
    add_test (NAME seg-urb00-xml-data COMMAND $<TARGET_FILE:dnasegmentwrapper> urban_xml --min 50 --max 50)
    add_test (NAME imp-urb07-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_xml ./urban_dnastn.xml ./urban_dnamsr.xml --import-block-stn-msr 2)
    add_test (NAME imp-urb08-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n urban_xml ./urban_dnastn.xml ./urban_dnamsr.xml --geo-file urban.geo)
    add_test (NAME plt-urb01-xml-data COMMAND $<TARGET_FILE:dnaplotwrapper> urban_xml --supress-pdf --centre-station 1004 --area-radius 5000 --plot-msr-types gxyasvz)
    add_test (NAME plt-urb02-xml-data COMMAND $<TARGET_FILE:dnaplotwrapper> urban_xml --supress-pdf --centre-latitude -37.480489013 --centre-longitude 144.573579814 --area-radius 3000 --omit-meas)
    add_test (NAME imp-psu01-dna-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n pseudo ${CMAKE_SOURCE_DIR}/../sampleData/pseudo.stn)
    add_test (NAME geo-psu00-dna-data COMMAND $<TARGET_FILE:dnageoidwrapper> pseudo -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb --export-dna-geo-file)
    add_test (NAME imp-psu02-dna-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n pseudo ${CMAKE_SOURCE_DIR}/../sampleData/pseudo.stn ${CMAKE_SOURCE_DIR}/../sampleData/pseudo-driver-file.msr --simulate --geo-file pseudo.geo --export-xml)
    add_test (NAME imp-psu03-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n pseudo pseudostn.xml pseudomsr.xml)
    add_test (NAME geo-psu01-xml-data COMMAND $<TARGET_FILE:dnageoidwrapper> pseudo -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb --convert)
    add_test (NAME adj-psu00-xml-data COMMAND $<TARGET_FILE:dnaadjustwrapper> pseudo --output-adj-msr --free-stn-sd 5.0 --fixed-stn-sd 0.000005 --max-iterations 15 --output-tstat-adj-msr --sort-adj-msr-field 7 --sort-stn-orig-order --stn-coord-types XYZPLHhENz --angular-stn-type 1 --angular-msr-type 1 --precision-stn-linear 3 --precision-msr-linear 3 --precision-stn-angular 4 --precision-msr-angular 4 --output-pos-uncertainty --output-all-covariances --output-corrections-file)
    add_test (NAME imp-gnss0-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n gnss_xml ./gnssstn.xml ./gnssmsr.xml --bounding-box -36.3000,145.4500,-36.4500,146.4500 --get-msrs-transcending-box --split-gnss-cluster-msrs --prefer-single-x-as-g)
    add_test (NAME imp-gnss1-xml-data COMMAND $<TARGET_FILE:dnaimportwrapper> -n gnss_xml ./gnssstn.xml ./gnssmsr.xml --include-stns-assoc-msrs "BEEC,BNLA,EURA,HOTH,MNSF,MYRT,320500750,385900240" --get-msrs-transcending-box --split-gnss-cluster-msrs --prefer-single-x-as-g -r itrf2014 --override-input-ref-frame)

    # 7. geoid data manipulation
    add_test (NAME geo-interpolate-01 COMMAND $<TARGET_FILE:dnageoidwrapper> -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb --interactive --latitude -36.3348253617 --longitude 145.5741006771)
    add_test (NAME geo-interpolate-02 COMMAND $<TARGET_FILE:dnageoidwrapper> -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb --interactive --latitude -36.2704890597 --longitude 146.4809871537 --interpolation-method 0)
    add_test (NAME geo-interpolate-03 COMMAND $<TARGET_FILE:dnageoidwrapper> -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb --summary)
    add_test (NAME geo-interpolate-04 COMMAND $<TARGET_FILE:dnageoidwrapper> --version)
    add_test (NAME geo-interpolate-05 COMMAND $<TARGET_FILE:dnageoidwrapper> -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb -t ${CMAKE_SOURCE_DIR}/../sampleData/geoid-points-dat.txt)
    add_test (NAME geo-interpolate-06 COMMAND $<TARGET_FILE:dnageoidwrapper> -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb -t ${CMAKE_SOURCE_DIR}/../sampleData/geoid-points-csv.csv)    
    add_test (NAME geo-ntv2-create-01 COMMAND $<TARGET_FILE:dnageoidwrapper> -d ${CMAKE_SOURCE_DIR}/../sampleData/ausgeoid09_gda94_v1.01_clip_1x1.dat -c -g ausgeoid_clip_default.gsb)
    add_test (NAME geo-ntv2-create-02 COMMAND $<TARGET_FILE:dnageoidwrapper> -d ${CMAKE_SOURCE_DIR}/../sampleData/ausgeoid09_gda94_v1.01_clip_1x1.dat -c -g ausgeoid_clip_1.0.1.0.gsb --grid-shift radians --grid-version 1.0.1.0 --system-fr ___GDA94 --system-to ___AHD71 --sub-grid-n 1D-grid --creation 21.04.2021 --update 22.04.2021)
    add_test (NAME geo-ntv2-export-01 COMMAND $<TARGET_FILE:dnageoidwrapper> -g ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network-geoid.gsb --export-ntv2-asc)
    add_test (NAME geo-ntv2-export-02 COMMAND $<TARGET_FILE:dnageoidwrapper> -g ./gnss-network-geoid.gsb.asc --export-ntv2-gsb)

    # bash command to check results
    add_test (NAME test-gnss-network COMMAND bash -c "diff <(tail -n +53 gnss.simult.adj) <(tail -n +53 ${CMAKE_SOURCE_DIR}/../sampleData/gnss.simult.adj.expected)")
    #add_test (NAME test-urban-phased-network COMMAND bash -c "diff <(tail -n +62 urban.phased.adj) <(tail -n +62 ${CMAKE_SOURCE_DIR}/../sampleData/urban.phased.adj.expected)")
    #add_test (NAME test-urban-thread-network COMMAND bash -c "diff <(tail -n +64 urban_mt.phased-mt.adj) <(tail -n +64 ${CMAKE_SOURCE_DIR}/../sampleData/urban_mt.phased-mt.adj.expected)")

    # 8. gnss reference frame transformations
    add_test (NAME ref-gnss01-network COMMAND $<TARGET_FILE:dnareftranwrapper> gnss -r itrf2014 -e 01.01.2020)
    add_test (NAME ref-gnss02-network COMMAND $<TARGET_FILE:dnareftranwrapper> gnss -r itrf1991 -e 03.12.1995)
    add_test (NAME ref-gnss03-network COMMAND $<TARGET_FILE:dnareftranwrapper> gnss -r itrf2005 -e 05.05.2005)
    add_test (NAME ref-gnss04-network COMMAND $<TARGET_FILE:dnareftranwrapper> gnss -r itrf1991 -e 05.05.2005)
    add_test (NAME ref-gnss05-network COMMAND $<TARGET_FILE:dnareftranwrapper> gnss -r gda94)

    # 9. gnss reference frame transformations with plate motion model
    add_test (NAME ref-itrf-pmm-01 COMMAND $<TARGET_FILE:dnaimportwrapper> -n itrf05_itrf1997_a -r itrf2005 ${CMAKE_SOURCE_DIR}/../sampleData/TEST_ITRF05.SNX)
    add_test (NAME ref-itrf-pmm-02 COMMAND $<TARGET_FILE:dnareftranwrapper> itrf05_itrf1997_a -r itrf1997 -e 25.10.2017 --export-dna --verb 2 --plate-model-option 1 -b ${CMAKE_SOURCE_DIR}/../sampleData/PB2002_plates.dig -m ${CMAKE_SOURCE_DIR}/../sampleData/PB2002_poles.dat)
    add_test (NAME ref-itrf-pmm-03 COMMAND $<TARGET_FILE:dnaimportwrapper> -n itrf05_itrf1997_b -r itrf2005 ${CMAKE_SOURCE_DIR}/../sampleData/TEST_ITRF05.SNX)
    add_test (NAME ref-itrf-pmm-04 COMMAND $<TARGET_FILE:dnareftranwrapper> itrf05_itrf1997_b -r itrf1997 -e 01.01.2020 --export-dna --verb 3 --plate-model-option 1 -b ${CMAKE_SOURCE_DIR}/../sampleData/PB2002_plates.dig -m ${CMAKE_SOURCE_DIR}/../sampleData/PB2002_poles.dat)
    add_test (NAME ref-itrf-pmm-05 COMMAND $<TARGET_FILE:dnaimportwrapper> -n apr -r itrf2008 ${CMAKE_SOURCE_DIR}/../sampleData/apr.ITRF2008.04.06.2020.stn ${CMAKE_SOURCE_DIR}/../sampleData/apr.ITRF2008.04.06.2020.msr)
    add_test (NAME ref-itrf-pmm-06 COMMAND $<TARGET_FILE:dnareftranwrapper> apr -r itrf2008 -e 01.01.2021 --export-dna --verb 2 --plate-model-option 1 -b ${CMAKE_SOURCE_DIR}/../sampleData/MORVEL56_plates.dig -m ${CMAKE_SOURCE_DIR}/../sampleData/NNR-MORVEL56_poles.dat)
    #add_test (NAME ref-itrf-pmm-07 COMMAND bash -c "diff <(tail -n +6 apr.ITRF2008.01.01.2021.stn) <(tail -n +4 ${CMAKE_SOURCE_DIR}/../sampleData/apr.ITRF2008.01.01.2021.stn.expected)")

    # 10. station discontinuities
    add_test (NAME imp-discont-01 COMMAND $<TARGET_FILE:dnaimportwrapper> -n discont -r itrf2005 ${CMAKE_SOURCE_DIR}/../sampleData/TEST_ITRF05.SNX --discontinuity-file ${CMAKE_SOURCE_DIR}/../sampleData/disconts20201205.snx)
    add_test (NAME imp-discont-02 COMMAND $<TARGET_FILE:dnaimportwrapper> -n discont -r itrf2005 ${CMAKE_SOURCE_DIR}/../sampleData/TEST_ITRF05.SNX --discontinuity-file ${CMAKE_SOURCE_DIR}/../sampleData/disconts20201205.snx --split-gnss --include-stn "ALIC" --export-dna)
    add_test (NAME imp-discont-03 COMMAND $<TARGET_FILE:dnaimportwrapper> -n discont -r itrf2014 --override-input-ref-frame ${CMAKE_SOURCE_DIR}/../sampleData/TEST_ITRF05.SNX ${CMAKE_SOURCE_DIR}/../sampleData/sa-gnss-stn.xml ${CMAKE_SOURCE_DIR}/../sampleData/sa-gnss-msr.xml --prefer-single-x-as-g --flag-unused --discontinuity-file ${CMAKE_SOURCE_DIR}/../sampleData/disconts20201205.snx --remove-ignored-msr --split-gnss --include-stns-assoc-msrs "ALIC" --export-dna --export-xml)
    
    # 11. measurements to stations sorting
    add_test (NAME imp-m2s-sort-01 COMMAND $<TARGET_FILE:dnaimportwrapper> -n gnss ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.stn ${CMAKE_SOURCE_DIR}/../sampleData/gnss-network.msr --output-msr-to-stn --sort-msr-to-stn-field 1 -r GDA94) 

    # 12. help modules and version
    add_test (NAME imp-help-01 COMMAND $<TARGET_FILE:dnaimportwrapper> --help-module standard)
    add_test (NAME imp-help-02 COMMAND $<TARGET_FILE:dnaimportwrapper> --help-module reference)
    add_test (NAME imp-help-03 COMMAND $<TARGET_FILE:dnaimportwrapper> --help-module screening)
    add_test (NAME imp-help-04 COMMAND $<TARGET_FILE:dnaimportwrapper> --help-module gnss)
    add_test (NAME imp-help-05 COMMAND $<TARGET_FILE:dnaimportwrapper> --help-module simulation)
    add_test (NAME imp-help-06 COMMAND $<TARGET_FILE:dnaimportwrapper> --help-module output)
    add_test (NAME imp-help-07 COMMAND $<TARGET_FILE:dnaimportwrapper> --help-module export)
    add_test (NAME imp-help-08 COMMAND $<TARGET_FILE:dnaimportwrapper> --help-module generic)
    add_test (NAME imp-version-01 COMMAND $<TARGET_FILE:dnaimportwrapper> --version)

    add_test (NAME geo-help-01 COMMAND $<TARGET_FILE:dnageoidwrapper> --help-module standard)
    add_test (NAME geo-help-02 COMMAND $<TARGET_FILE:dnageoidwrapper> --help-module interpolation)
    add_test (NAME geo-help-03 COMMAND $<TARGET_FILE:dnageoidwrapper> --help-module ntv2)
    add_test (NAME geo-help-04 COMMAND $<TARGET_FILE:dnageoidwrapper> --help-module interactive)
    add_test (NAME geo-help-05 COMMAND $<TARGET_FILE:dnageoidwrapper> --help-module file)
    add_test (NAME geo-help-06 COMMAND $<TARGET_FILE:dnageoidwrapper> --help-module export)
    add_test (NAME geo-help-07 COMMAND $<TARGET_FILE:dnageoidwrapper> --help-module generic)
    add_test (NAME geo-version-01 COMMAND $<TARGET_FILE:dnageoidwrapper> --version)

    add_test (NAME seg-help-01 COMMAND $<TARGET_FILE:dnasegmentwrapper> --help-module standard)
    add_test (NAME seg-help-02 COMMAND $<TARGET_FILE:dnasegmentwrapper> --help-module configuration)
    add_test (NAME seg-help-03 COMMAND $<TARGET_FILE:dnasegmentwrapper> --help-module generic)
    add_test (NAME seg-version-01 COMMAND $<TARGET_FILE:dnasegmentwrapper> --version)

    add_test (NAME ref-help-01 COMMAND $<TARGET_FILE:dnareftranwrapper> --help-module standard)
    add_test (NAME ref-help-02 COMMAND $<TARGET_FILE:dnareftranwrapper> --help-module transformation)
    add_test (NAME ref-help-03 COMMAND $<TARGET_FILE:dnareftranwrapper> --help-module export)
    add_test (NAME ref-help-04 COMMAND $<TARGET_FILE:dnareftranwrapper> --help-module generic)
    add_test (NAME ref-version-01 COMMAND $<TARGET_FILE:dnareftranwrapper> --version)

    add_test (NAME adj-help-01 COMMAND $<TARGET_FILE:dnaadjustwrapper> --help-module standard)
    add_test (NAME adj-help-02 COMMAND $<TARGET_FILE:dnaadjustwrapper> --help-module adjustment)
    add_test (NAME adj-help-03 COMMAND $<TARGET_FILE:dnaadjustwrapper> --help-module phased)
    add_test (NAME adj-help-04 COMMAND $<TARGET_FILE:dnaadjustwrapper> --help-module configuration)
    add_test (NAME adj-help-05 COMMAND $<TARGET_FILE:dnaadjustwrapper> --help-module stage)
    add_test (NAME adj-help-06 COMMAND $<TARGET_FILE:dnaadjustwrapper> --help-module output)
    add_test (NAME adj-help-07 COMMAND $<TARGET_FILE:dnaadjustwrapper> --help-module export)
    add_test (NAME adj-help-08 COMMAND $<TARGET_FILE:dnaadjustwrapper> --help-module generic)
    add_test (NAME adj-version-01 COMMAND $<TARGET_FILE:dnaadjustwrapper> --version)

    add_test (NAME plt-help-01 COMMAND $<TARGET_FILE:dnaplotwrapper> --help-module standard)
    add_test (NAME plt-help-02 COMMAND $<TARGET_FILE:dnaplotwrapper> --help-module data)
    add_test (NAME plt-help-03 COMMAND $<TARGET_FILE:dnaplotwrapper> --help-module mapping)
    add_test (NAME plt-help-04 COMMAND $<TARGET_FILE:dnaplotwrapper> --help-module pdf)
    add_test (NAME plt-help-05 COMMAND $<TARGET_FILE:dnaplotwrapper> --help-module generic)
    add_test (NAME plt-version-01 COMMAND $<TARGET_FILE:dnaplotwrapper> --version)

    add_test (NAME dyn-help-01 COMMAND $<TARGET_FILE:dynadjust> --help-module standard)
    add_test (NAME dyn-help-02 COMMAND $<TARGET_FILE:dynadjust> --help-module generic)
    add_test (NAME dyn-version-01 COMMAND $<TARGET_FILE:dynadjust> --version)

    # 13. database id
    add_test (NAME import-dbid COMMAND $<TARGET_FILE:dnaimportwrapper> -n dbid ${CMAKE_SOURCE_DIR}/../sampleData/dsg.stn ${CMAKE_SOURCE_DIR}/../sampleData/dsg.msr) 
    add_test (NAME adjust-dbid COMMAND $<TARGET_FILE:dnaadjustwrapper> dbid --output-adj-msr --output-database-ids --output-ignored-msrs --sort-adj-msr-field 6)
    
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
    set_tests_properties(ref-itrf-pmm-06 PROPERTIES DEPENDS ref-itrf-pmm-05)
    #set_tests_properties(ref-itrf-pmm-07 PROPERTIES DEPENDS ref-itrf-pmm-06)

endif ()

message (" ")
message ("===========================================================================")
message ("Finished configuring DynAdjust ${DYNADJUST_VERSION} (${CMAKE_BUILD_TYPE}).")
message (" ")
