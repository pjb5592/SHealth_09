cmake_minimum_required(VERSION 3.10)
project(CoreProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(SHEALTH_COVERAGE "Enable gcc/gcov coverage for shealth_lib and SHealthBMITest" OFF)

function(shealth_enable_coverage target)
    target_compile_options(${target} PUBLIC --coverage -O0 -g)
    target_link_options(${target} PUBLIC --coverage)
endfunction()

# Main library
add_library(shealth_lib
    src/main/cpp/SHealth.cpp
    src/main/cpp/CsvLoader.cpp
    src/main/cpp/Imputation.cpp
    src/main/cpp/Statistics.cpp
    src/main/cpp/SHealthBmiReport.cpp
)
target_include_directories(shealth_lib PUBLIC src/main/cpp)
if(SHEALTH_COVERAGE)
    shealth_enable_coverage(shealth_lib)
    # --coverage propagates to SHealthBMI / SHealthBMITest via PUBLIC link
endif()

# Main executable
add_executable(SHealthBMI src/main/cpp/SHealthBMI.cpp)
target_link_libraries(SHealthBMI PRIVATE shealth_lib)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Test executable
add_executable(SHealthBMITest src/test/cpp/SHealthBMITest.cpp)
target_link_libraries(SHealthBMITest PRIVATE shealth_lib GTest::gtest_main)
target_compile_definitions(SHealthBMITest PRIVATE
    SHEALTH_FIXTURE_DIR="${CMAKE_SOURCE_DIR}/src/test/fixtures"
    SHEALTH_PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
    SHEALTH_GOLDEN_DIR="${CMAKE_SOURCE_DIR}/test/golden"
)

set(SHEALTH_GOLDEN_FILE "${CMAKE_SOURCE_DIR}/test/golden/shealth_bmi.approved.txt")
add_custom_target(update-golden-shealth-bmi
    COMMAND ${CMAKE_COMMAND}
        -DPROJECT_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DEXE=$<TARGET_FILE:SHealthBMI>
        -DGOLDEN_FILE=${SHEALTH_GOLDEN_FILE}
        -P ${CMAKE_SOURCE_DIR}/cmake/UpdateGolden.cmake
    DEPENDS SHealthBMI
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Regenerate test/golden/shealth_bmi.approved.txt from SHealthBMI"
)

include(GoogleTest)
gtest_discover_tests(SHealthBMITest WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

if(SHEALTH_COVERAGE)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)
    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND}
                -DCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
                -DSHEALTH_SOURCE_DIR=${CMAKE_SOURCE_DIR}
                -P ${CMAKE_SOURCE_DIR}/cmake/GenerateCoverage.cmake
            DEPENDS SHealthBMITest
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "ctest + lcov + genhtml -> build/coverage_html"
        )
    else()
        message(WARNING "SHEALTH_COVERAGE=ON but lcov/genhtml not found; coverage target skipped")
    endif()
endif()
