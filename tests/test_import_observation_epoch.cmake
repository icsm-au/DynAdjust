cmake_minimum_required(VERSION 3.16)

set(work_dir "${TEST_DIR}/${TEST_MODE}")
file(MAKE_DIRECTORY "${work_dir}")

function(run_checked)
    execute_process(COMMAND ${ARGV}
        WORKING_DIRECTORY "${work_dir}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
        TIMEOUT 30)
    if(NOT result STREQUAL "0")
        message(FATAL_ERROR "${ARGV} failed (${result}):\n${output}\n${error}")
    endif()
endfunction()

function(pad value width result)
    string(LENGTH "${value}" length)
    math(EXPR padding "${width} - ${length}")
    string(REPEAT " " ${padding} spaces)
    set(${result} "${value}${spaces}" PARENT_SCOPE)
endfunction()

function(linear_record type first target frame_epoch observation_epoch result)
    pad("${first}" 20 instrument)
    pad("${target}" 20 target1)
    pad("" 20 target2)
    pad("100.0" 14 value)
    pad("" 14 angles)
    pad("0.01" 9 sd)
    pad("" 23 gap)
    pad("${frame_epoch}" 20 epoch)
    pad("" 20 ids)
    set(${result} "${type} ${instrument}${target1}${target2}${value}${angles}${sd}${gap}${epoch}${ids}${observation_epoch}\n" PARENT_SCOPE)
endfunction()

function(check_epochs filename)
    file(STRINGS "${work_dir}/${filename}" records REGEX "^[LH] ")
    set(expected ${ARGN})
    list(LENGTH records count)
    list(LENGTH expected expected_count)
    if(NOT count EQUAL expected_count)
        message(FATAL_ERROR "${filename}: expected ${expected_count} measurements, got ${count}")
    endif()
    foreach(record IN LISTS records)
        string(LENGTH "${record}" length)
        if(length LESS 172)
            message(FATAL_ERROR "${filename}: observation-epoch column missing: ${record}")
        endif()
        string(SUBSTRING "${record}" 162 -1 actual)
        string(STRIP "${actual}" actual)
        list(FIND expected "${actual}" index)
        if(index EQUAL -1)
            message(FATAL_ERROR "${filename}: unexpected epoch '${actual}', expected ${expected}")
        endif()
        list(REMOVE_AT expected ${index})
    endforeach()
endfunction()

file(WRITE "${work_dir}/stations.xml" [=[<?xml version="1.0"?>
<DnaXmlFormat type="Station File" referenceframe="GDA2020" epoch="01.01.2020">
  <DnaStation><Name>ALIC</Name><Constraints>CCC</Constraints><Type>XYZ</Type>
    <StationCoord><Name>ALIC</Name><XAxis>-4052052.7379</XAxis><YAxis>4212835.9824</YAxis><Height>-2545104.5871</Height></StationCoord>
  </DnaStation>
  <DnaStation><Name>WARA</Name><Constraints>FFF</Constraints><Type>XYZ</Type>
    <StationCoord><Name>WARA</Name><XAxis>-3583757.5894</XAxis><YAxis>4538446.7027</YAxis><Height>-2683056.6932</Height></StationCoord>
  </DnaStation>
</DnaXmlFormat>
]=])
linear_record(L ALIC WARA "" "" missing)
linear_record(H ALIC "" "01.01.2020" "01.01.2020" explicit_equal)
linear_record(H WARA "" "01.01.2020" "17.07.2018" explicit_distinct)
file(WRITE "${work_dir}/mixed.msr"
    "!#=DNA 3.02 MSR    29.09.2026    GDA2020     01.01.2020         3\n${missing}${explicit_equal}${explicit_distinct}")
run_checked("${IMPORT_EXE}" -n mixed stations.xml mixed.msr)
run_checked("${SEGMENT_EXE}" mixed --contiguous-blocks 0)

if(TEST_MODE STREQUAL "block")
    set(extraction_option --import-block-stn-msr 1)
    set(export_name mixed.block-1.msr)
elseif(TEST_MODE STREQUAL "network")
    set(extraction_option --import-contiguous-stn-msr 0)
    set(export_name mixed.network-0.msr)
else()
    message(FATAL_ERROR "Unknown TEST_MODE: ${TEST_MODE}")
endif()

run_checked("${IMPORT_EXE}" -n mixed ${extraction_option} --export-dna)
file(STRINGS "${work_dir}/${export_name}" undated REGEX "^L ")
string(SUBSTRING "${undated}" 162 -1 observation_epoch)
string(STRIP "${observation_epoch}" observation_epoch)
if(NOT observation_epoch STREQUAL "")
    message(FATAL_ERROR "Extraction supplied an unrequested observation epoch")
endif()

run_checked("${IMPORT_EXE}" -n mixed ${extraction_option}
    --observation-epoch 15.06.2015 --export-dna)
check_epochs("${export_name}" 15.06.2015 01.01.2020 17.07.2018)

run_checked("${IMPORT_EXE}" -n reimported stations.xml "${export_name}"
    --observation-epoch 01.01.2025 --export-dna)
check_epochs(reimported.msr 15.06.2015 01.01.2020 17.07.2018)
