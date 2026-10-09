cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED IMPORT_EXE OR NOT EXISTS "${IMPORT_EXE}")
    message(FATAL_ERROR "Set IMPORT_EXE to the full path of an existing dnaimport executable")
endif()
get_filename_component(IMPORT_EXE "${IMPORT_EXE}" ABSOLUTE)
if(NOT DEFINED OUTPUT_DIR)
    set(OUTPUT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../build/alic-epoch-repro")
endif()
get_filename_component(OUTPUT_DIR "${OUTPUT_DIR}" ABSOLUTE)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
set(ENV{OPENBLAS_NUM_THREADS} 1)

function(check_import name input expected_station expected_epoch)
    set(work_dir "${OUTPUT_DIR}/${name}")
    file(MAKE_DIRECTORY "${work_dir}")
    execute_process(
        COMMAND "${IMPORT_EXE}" -n alic "${input}"
            --discontinuity-file "${CMAKE_CURRENT_LIST_DIR}/alic-discontinuities.snx"
            --export-xml-files ${ARGN}
        WORKING_DIRECTORY "${work_dir}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
        TIMEOUT 30)
    file(WRITE "${work_dir}/import.log" "${output}\n${error}")
    if(NOT result STREQUAL "0")
        message(FATAL_ERROR "${name}: dnaimport failed (${result}). See ${work_dir}/import.log")
    endif()

    file(READ "${work_dir}/alicmsr.xml" exported)
    string(REGEX MATCHALL "<DnaMeasurement>" measurements "${exported}")
    list(LENGTH measurements count)
    if(NOT count EQUAL 1)
        message(FATAL_ERROR "${name}: expected exactly one exported measurement, got ${count}")
    endif()
    string(REGEX MATCH "<First>([^<]+)</First>" station_element "${exported}")
    set(actual_station "${CMAKE_MATCH_1}")
    message(STATUS "${name}: expected ${expected_station}; got ${actual_station}")
    if(NOT actual_station STREQUAL expected_station)
        message(FATAL_ERROR
            "Observation-epoch regression reproduced: the ${expected_epoch} observation was assigned "
            "to ${actual_station} instead of ${expected_station}. "
            "See ${work_dir}/alicmsr.xml and ${work_dir}/import.log")
    endif()
    if(NOT exported MATCHES "<Epoch>01[.]01[.]2020</Epoch>")
        message(FATAL_ERROR "${name}: the reference-frame epoch must remain 01.01.2020")
    endif()
    string(FIND "${exported}" "<EpochOfObservation>${expected_epoch}</EpochOfObservation>" epoch_pos)
    if(epoch_pos EQUAL -1)
        message(FATAL_ERROR "${name}: XML export lost observation epoch ${expected_epoch}")
    endif()
endfunction()

check_import(cli-default "${CMAKE_CURRENT_LIST_DIR}/alic.xml" ALIC_1900001 01.01.2010
    --observation-epoch 01.01.2010)

check_import(frame-epoch "${CMAKE_CURRENT_LIST_DIR}/alic.xml" ALIC_2011201 01.01.2020)
file(READ "${CMAKE_CURRENT_LIST_DIR}/alic.xml" input)
string(REPLACE "<First>ALIC</First>"
    "<EpochOfObservation>01.01.2010</EpochOfObservation>\n    <First>ALIC</First>"
    explicit_input "${input}")
file(WRITE "${OUTPUT_DIR}/explicit-epoch.xml" "${explicit_input}")
check_import(explicit-epoch "${OUTPUT_DIR}/explicit-epoch.xml" ALIC_1900001 01.01.2010)

check_import(explicit-with-default "${OUTPUT_DIR}/explicit-epoch.xml" ALIC_1900001 01.01.2010
    --observation-epoch 01.01.2025)
string(REPLACE "<EpochOfObservation>01.01.2010</EpochOfObservation>"
    "<EpochOfObservation>01.01.2020</EpochOfObservation>" equal_input "${explicit_input}")
file(WRITE "${OUTPUT_DIR}/explicit-frame-epoch.xml" "${equal_input}")
check_import(explicit-frame-epoch "${OUTPUT_DIR}/explicit-frame-epoch.xml" ALIC_2011201 01.01.2020
    --observation-epoch 01.01.2010)
message(STATUS "PASS: CLI observation epoch selects ALIC_1900001 and survives XML export")
