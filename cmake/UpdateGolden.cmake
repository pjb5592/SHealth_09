# Regenerate test/golden/shealth_bmi.approved.txt from SHealthBMI (project root cwd).
#
# Usage (from build/):
#   cmake --build . --target update-golden-shealth-bmi
#
# Required variables: PROJECT_SOURCE_DIR, EXE, GOLDEN_FILE

if(NOT PROJECT_SOURCE_DIR OR NOT EXE OR NOT GOLDEN_FILE)
    message(FATAL_ERROR "UpdateGolden.cmake requires PROJECT_SOURCE_DIR, EXE, GOLDEN_FILE")
endif()

get_filename_component(_golden_dir "${GOLDEN_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${_golden_dir}")

execute_process(
    COMMAND "${EXE}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_FILE "${GOLDEN_FILE}"
    ERROR_VARIABLE _stderr
    RESULT_VARIABLE _result
)

if(NOT _result EQUAL 0)
    message(FATAL_ERROR "SHealthBMI failed (code ${_result}): ${_stderr}")
endif()

message(STATUS "Golden master updated: ${GOLDEN_FILE}")
