cmake_minimum_required(VERSION 3.20)
project(x360ue3 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(X360PORT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../x360port" CACHE PATH
    "The consumed shared/x360port checkout supplying guest-address vocabulary")
if(NOT EXISTS "${X360PORT_ROOT}/include/x360port/module_contract.hpp")
    message(FATAL_ERROR
        "x360ue3: X360PORT_ROOT lacks include/x360port/module_contract.hpp: ${X360PORT_ROOT}")
endif()

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format REQUIRED)
find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)

add_library(x360ue3
    src/engine_contract.cpp
    include/x360ue3/engine_contract.hpp)
add_library(x360ue3::x360ue3 ALIAS x360ue3)
target_include_directories(x360ue3
    PUBLIC include
    PUBLIC "${X360PORT_ROOT}/include")
target_compile_features(x360ue3 PUBLIC cxx_std_20)
target_compile_options(x360ue3 PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    enable_testing()
    add_executable(x360ue3_contract_tests tests/contract_tests.cpp)
    target_link_libraries(x360ue3_contract_tests PRIVATE x360ue3)
    target_include_directories(x360ue3_contract_tests PRIVATE "${X360PORT_ROOT}/include")
    target_compile_options(x360ue3_contract_tests PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow>)
    add_test(NAME contract COMMAND x360ue3_contract_tests)
    add_test(NAME format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run --Werror
                src/engine_contract.cpp include/x360ue3/engine_contract.hpp
                tests/contract_tests.cpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME clang_tidy
        COMMAND ${CLANG_TIDY_EXECUTABLE} -p=${CMAKE_BINARY_DIR}
                --warnings-as-errors=*
                src/engine_contract.cpp tests/contract_tests.cpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
