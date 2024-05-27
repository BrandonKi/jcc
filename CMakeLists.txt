cmake_minimum_required(VERSION 3.20.0)
project(jcc VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 23)

cmake_policy(SET CMP0069 NEW)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_subdirectory(jab)

add_library(jcc_lib OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/llvm_ir_gen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sema.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ppc.cpp
)

target_include_directories(jcc_lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib

    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/jab/include
)

set(cmd_driver_src
	$<TARGET_OBJECTS:jab_lib>
	$<TARGET_OBJECTS:jcc_lib>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_executable(jcc ${cmd_driver_src})

target_include_directories(jcc
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib

    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/jab/include
)

llvm_map_components_to_libnames(llvm_libs support core irreader native)
# llvm_map_components_to_libnames(llvm_libs all)
target_link_libraries(jcc ${llvm_libs})

set(test_driver_src
	$<TARGET_OBJECTS:jab_lib>
    $<TARGET_OBJECTS:jcc_lib>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.cpp
)

add_executable(jcc_tests ${test_driver_src})

target_include_directories(jcc_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib
    
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/jab/include
)

target_link_libraries(jcc_tests ${llvm_libs})

if(${LTO})
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options(jcc PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(jcc_tests PRIVATE -Wall -Wextra -pedantic)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(jcc PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(jcc_tests PRIVATE -Wall -Wextra -pedantic)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    target_compile_options(jcc PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(jcc_tests PRIVATE -Wall -Wextra -pedantic)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(jcc PRIVATE /W3 /wd4624)
    target_compile_options(jcc_tests PRIVATE /W3 /wd4624)
endif()