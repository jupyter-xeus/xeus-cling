############################################################################
# Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.4.3)
project(xeus-cling)

#######################
# Rely on llvm-config #
#######################

set(CONFIG_OUTPUT)
find_program(LLVM_CONFIG "llvm-config")
if(LLVM_CONFIG)
    message(STATUS "Found LLVM_CONFIG as ${LLVM_CONFIG}")
    set(CONFIG_COMMAND ${LLVM_CONFIG}
        "--assertion-mode"
        "--bindir"
        "--libdir"
        "--includedir"
        "--prefix"
        "--src-root")
    execute_process(COMMAND ${CONFIG_COMMAND}
                    RESULT_VARIABLE HAD_ERROR
                    OUTPUT_VARIABLE CONFIG_OUTPUT)
    if(NOT HAD_ERROR)
        string(REGEX REPLACE
               "[ \t]*[\r\n]+[ \t]*" ";"
               CONFIG_OUTPUT ${CONFIG_OUTPUT})
    else()
        string(REPLACE ";" " " CONFIG_COMMAND_STR "${CONFIG_COMMAND}")
        message(STATUS "${CONFIG_COMMAND_STR}")
        message(FATAL_ERROR "llvm-config failed with status ${HAD_ERROR}")
    endif()
else()
    message(FATAL_ERROR "llvm-config not found -- ${LLVM_CONFIG}")
endif()

list(GET CONFIG_OUTPUT 0 ENABLE_ASSERTIONS)
list(GET CONFIG_OUTPUT 1 TOOLS_BINARY_DIR)
list(GET CONFIG_OUTPUT 2 LIBRARY_DIR)
list(GET CONFIG_OUTPUT 3 INCLUDE_DIR)
list(GET CONFIG_OUTPUT 4 LLVM_OBJ_ROOT)
list(GET CONFIG_OUTPUT 5 MAIN_SRC_DIR)

if(NOT MSVC_IDE)
    set(LLVM_ENABLE_ASSERTIONS ${ENABLE_ASSERTIONS} CACHE BOOL "Enable assertions")
    mark_as_advanced(LLVM_ENABLE_ASSERTIONS)
endif()

set(LLVM_TOOLS_BINARY_DIR ${TOOLS_BINARY_DIR} CACHE PATH "Path to llvm/bin")
set(LLVM_LIBRARY_DIR ${LIBRARY_DIR} CACHE PATH "Path to llvm/lib")
set(LLVM_MAIN_INCLUDE_DIR ${INCLUDE_DIR} CACHE PATH "Path to llvm/include")
set(LLVM_BINARY_DIR ${LLVM_OBJ_ROOT} CACHE PATH "Path to LLVM build tree")
set(LLVM_MAIN_SRC_DIR ${MAIN_SRC_DIR} CACHE PATH "Path to LLVM source tree")

link_directories(${LLVM_LIBRARY_DIR})
add_definitions(-DLLVM_DIR="${LLVM_BINARY_DIR}")

################
# Dependencies #
################

find_package(xeus REQUIRED)
include_directories(${xeus_INCLUDE_DIRS})

find_package(Clang REQUIRED)
find_package(Cling REQUIRED)
find_package(cxxopts REQUIRED)

#########
# flags #
#########

include(CheckCXXCompilerFlag)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

if(MSVC)
    add_definitions(/DNOMINMAX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4141")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4018 /wd4267 /wd4715 /wd4146 /wd4129")
endif()

OPTION(DISABLE_ARCH_NATIVE "disable -march=native flag" OFF)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-parameter -Wextra -Wreorder")
    if (DISABLE_ARCH_NATIVE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=generic")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    endif()

    CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)

    if (HAS_CPP14_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    else()
        message(FATAL_ERROR "Unsupported compiler -- xeus requires C++14 support!")
    endif()
endif()

###################
# Target and link #
###################

include(GNUInstallDirs)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
   set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

set(XEUSCLING_SRC
    src/main.cpp
    src/xbuffer.hpp
    src/xcpp_interpreter.cpp
    src/xcpp_interpreter.hpp
    src/xdemangle.hpp
    src/xoptions.cpp
    src/xoptions.hpp
    src/xparser.cpp
    src/xparser.hpp
    src/xholder_cling.cpp
    src/xholder_cling.hpp
    src/xmagics.hpp
    src/xmagics/execution.cpp
    src/xmagics/execution.hpp
    src/xmagics/os.cpp
    src/xmagics/os.hpp
)
set(LLVM_NO_DEAD_STRIP 1)
#set(LIBS ${xeus_LIBRARY} LLVMSupport clangFrontendTool clingUserInterface clingInterpreter clingMetaProcessor clingUtils pugixml)
set(LIBS ${xeus_LIBRARY} clingInterpreter clingMetaProcessor clingUtils pugixml)
set(XEUSCLING_TARGET xeus-cling)
add_executable(${XEUSCLING_TARGET} ${XEUSCLING_SRC})
set_target_properties(${XEUSCLING_TARGET} PROPERTIES ENABLE_EXPORTS 1)

if(MSVC)
  set_target_properties(${XEUSCLING_TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)  # Internal string
endif(MSVC)

target_link_libraries(${XEUSCLING_TARGET} ${LIBS})

################
# Installation #
################

install(TARGETS ${XEUSCLING_TARGET}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

set(KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/kernels)
set(XEUS_KERNELSPEC_INSTALL_DIR "share/jupyter" CACHE STRING "install path for kernel specs")

install(DIRECTORY ${KERNELSPEC_DIR}
        DESTINATION ${XEUS_KERNELSPEC_INSTALL_DIR})

set(TAGFILESPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tagfile)
set(XEUS_TAGFILESPEC_INSTALL_DIR "share/jupyter" CACHE STRING "install path for kernel specs")

install(DIRECTORY ${TAGFILESPEC_DIR}
        DESTINATION ${XEUS_TAGFILESPEC_INSTALL_DIR})
add_definitions(-DTAGFILE_DIR="${CMAKE_INSTALL_PREFIX}/${XEUS_TAGFILESPEC_INSTALL_DIR}/tagfile")
