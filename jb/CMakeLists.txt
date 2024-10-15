cmake_minimum_required(VERSION 3.20.0)
project(jb VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 23)

cmake_policy(SET CMP0069 NEW)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_library(jb_lib OBJECT
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jb.cpp	
    ${CMAKE_CURRENT_SOURCE_DIR}/src/module_builder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/jit_env.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/context.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/baseline_interp.cpp


# Register stuff
	${CMAKE_CURRENT_SOURCE_DIR}/src/register_manager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/register_allocator.cpp

# JBIR Analysis stuff
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/analysis/create_cfg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/analysis/create_dom_tree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/analysis/liveness.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/analysis/cfg_viz.cpp

# JBIR Optimization stuff
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/opt/mem2reg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/opt/phi_elim.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/opt/dce.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/opt/sscp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/opt/gvn.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/JBIR/opt/peephole.cpp

# MCIR Analysis stuff
	${CMAKE_CURRENT_SOURCE_DIR}/src/passes/MCIR/analysis/live_range.cpp

# MCIR Optimization stuff
	# ${CMAKE_CURRENT_SOURCE_DIR}/src/passes/MCIR/opt/ 

# Object File stuff
	${CMAKE_CURRENT_SOURCE_DIR}/src/link/coff.cpp

# Linking stuff
	${CMAKE_CURRENT_SOURCE_DIR}/src/link/windows_pe.cpp

# x86_64 stuff
	${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/mcir.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/mcir_gen.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/encode.cpp

# Misc. stuff
	${CMAKE_CURRENT_SOURCE_DIR}/src/misc/crc.cpp

)

target_include_directories(jb_lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    # PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/passes
    
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)

set(cmd_driver_src
	$<TARGET_OBJECTS:jb_lib>
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_executable(jb ${cmd_driver_src})

target_include_directories(jb
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    # PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/passes
    
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)

set(test_driver_src
	$<TARGET_OBJECTS:jb_lib>
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.cpp
)

add_executable(jb_tests ${test_driver_src})

target_include_directories(jb_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    # PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/passes
    
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib
)

if(${LTO})
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    target_compile_options(jb PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(jb_tests PRIVATE -Wall -Wextra -pedantic)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(jb PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(jb_tests PRIVATE -Wall -Wextra -pedantic)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    target_compile_options(jb PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(jb_tests PRIVATE -Wall -Wextra -pedantic)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(jb PRIVATE /W3 /wd4624)
    target_compile_options(jb_tests PRIVATE /W3 /wd4624)
endif()

