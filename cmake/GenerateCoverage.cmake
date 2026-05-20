# Run: cmake --build build --target coverage  (requires -DSHEALTH_COVERAGE=ON)
cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED CMAKE_BINARY_DIR)
  message(FATAL_ERROR "GenerateCoverage.cmake must run from a configured build tree")
endif()
if(NOT DEFINED SHEALTH_SOURCE_DIR)
  message(FATAL_ERROR "SHEALTH_SOURCE_DIR required (pass -DSHEALTH_SOURCE_DIR=<project root>)")
endif()

set(_obj_dir "${CMAKE_BINARY_DIR}/CMakeFiles/shealth_lib.dir/src/main/cpp")
set(COVERAGE_SUMMARY "${CMAKE_BINARY_DIR}/coverage_summary.txt")

find_program(PYTHON_EXECUTABLE NAMES python3 python)

message(STATUS "Running tests before coverage capture...")
execute_process(
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  RESULT_VARIABLE _ctest_result
)
if(_ctest_result)
  message(FATAL_ERROR "ctest failed (exit ${_ctest_result}); fix tests before coverage")
endif()

if(NOT IS_DIRECTORY "${_obj_dir}")
  message(FATAL_ERROR "Object directory not found: ${_obj_dir}")
endif()

if(NOT PYTHON_EXECUTABLE)
  message(FATAL_ERROR "python not found; required to summarize gcov (GCC 15 JSON vs lcov)")
endif()

message(STATUS "Summarizing gcov from ${_obj_dir} ...")
execute_process(
  COMMAND ${PYTHON_EXECUTABLE}
    "${SHEALTH_SOURCE_DIR}/cmake/SummarizeGcov.py"
    --object-dir ${_obj_dir}
    --summary-out ${COVERAGE_SUMMARY}
  RESULT_VARIABLE _py_result
)
if(_py_result)
  message(FATAL_ERROR "SummarizeGcov.py failed (${_py_result})")
endif()

# Optional HTML via lcov (may fail on GCC 15 JSON gcov — gcov text summary remains authoritative)
find_program(LCOV_EXECUTABLE lcov)
find_program(GENHTML_EXECUTABLE genhtml)
set(COVERAGE_INFO "${CMAKE_BINARY_DIR}/coverage.info")
set(COVERAGE_FILTERED "${CMAKE_BINARY_DIR}/coverage.filtered.info")
set(COVERAGE_HTML_DIR "${CMAKE_BINARY_DIR}/coverage_html")

if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
  message(STATUS "Attempting lcov HTML report (optional)...")
  execute_process(
    COMMAND ${LCOV_EXECUTABLE}
      --directory "${CMAKE_BINARY_DIR}"
      --capture
      --output-file "${COVERAGE_INFO}"
      --rc branch_coverage=1
    RESULT_VARIABLE _lcov_cap
  )
  if(_lcov_cap EQUAL 0)
    execute_process(
      COMMAND ${LCOV_EXECUTABLE}
        --remove "${COVERAGE_INFO}"
        "*/googletest/*"
        "*/_deps/*"
        "*/src/test/*"
        "*/test/cpp/*"
        "*/usr/*"
        "*/mingw64/*"
        "*/include/c++/*"
        --output-file "${COVERAGE_FILTERED}"
        --rc branch_coverage=1
      RESULT_VARIABLE _lcov_rm
    )
    if(_lcov_rm EQUAL 0)
      file(MAKE_DIRECTORY "${COVERAGE_HTML_DIR}")
      execute_process(
        COMMAND ${GENHTML_EXECUTABLE}
          "${COVERAGE_FILTERED}"
          --output-directory "${COVERAGE_HTML_DIR}"
          --branch-coverage
          --legend
          --title "SHealth shealth_lib coverage"
        RESULT_VARIABLE _genhtml
      )
      if(_genhtml EQUAL 0)
        message(STATUS "HTML report: ${COVERAGE_HTML_DIR}/index.html")
      else()
        message(STATUS "genhtml failed; use ${COVERAGE_SUMMARY} (gcov aggregate)")
      endif()
    endif()
  else()
    message(STATUS "lcov --capture skipped (GCC 15 gcov JSON may be incompatible); see ${COVERAGE_SUMMARY}")
  endif()
endif()

message(STATUS "Coverage summary: ${COVERAGE_SUMMARY}")
