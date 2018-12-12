############################################################################
# Copyright (c) 2016, Johan Mabille, Loic Gouarin and Sylvain Corlay       #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.4.3)
project(xeus-cling)

set(XCPP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Versionning
# ===========

file(STRINGS "${XCPP_SRC_DIR}/xcpp_config.hpp" xcpp_version_defines
     REGEX "#define XCPP_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xcpp_version_defines})
    if(ver MATCHES "#define XCPP_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XCPP_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
    ${XCPP_VERSION_MAJOR}.${XCPP_VERSION_MINOR}.${XCPP_VERSION_PATCH})
message(STATUS "Building xeus-cling v${${PROJECT_NAME}_VERSION}")

# Configuration
# =============

include(GNUInstallDirs)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xeus-cling-cpp11/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xeus-cling-cpp11/kernel.json"
)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xeus-cling-cpp14/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xeus-cling-cpp14/kernel.json"
)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xeus-cling-cpp17/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xeus-cling-cpp17/kernel.json"
)

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

find_package(xeus 0.18.1 REQUIRED)
find_package(cppzmq 4.3.0 REQUIRED)
find_package(pugixml REQUIRED)

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

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib; ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

set(XEUSCLING_SRC
    src/main.cpp
    src/xbuffer.hpp
    src/xinput.hpp
    src/xinput.cpp
    src/xinterpreter.cpp
    src/xinterpreter.hpp
    src/xdemangle.hpp
    src/xoptions.cpp
    src/xoptions.hpp
    src/xcpp_config.hpp
    src/xparser.cpp
    src/xparser.hpp
    src/xholder_cling.cpp
    src/xholder_cling.hpp
    src/xmagics.hpp
    src/xmagics/execution.cpp
    src/xmagics/execution.hpp
    src/xmagics/os.cpp
    src/xmagics/os.hpp
    src/xmime_internal.hpp
)

set(XCPP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set (XCPP_HEADERS
   ${XCPP_INCLUDE_DIR}/xcpp/xmime.hpp
   ${XCPP_INCLUDE_DIR}/xcpp/xdisplay.hpp
)

set(LLVM_NO_DEAD_STRIP 1)
set(LIBS clingInterpreter clingMetaProcessor clingUtils xeus pugixml cxxopts::cxxopts)
set(XEUSCLING_TARGET xeus-cling)
add_executable(${XEUSCLING_TARGET} ${XEUSCLING_SRC})
set_target_properties(${XEUSCLING_TARGET} PROPERTIES ENABLE_EXPORTS 1)

target_include_directories(${XEUSCLING_TARGET} PRIVATE ${XCPP_INCLUDE_DIR})

if (APPLE)
    set_target_properties(${XEUSCLING_TARGET} PROPERTIES
        MACOSX_RPATH ON
    )
else()
    set_target_properties(${XEUSCLING_TARGET} PROPERTIES
        BUILD_WITH_INSTALL_RPATH 1
        SKIP_BUILD_RPATH FALSE
    )
endif()

set_target_properties(${XEUSCLING_TARGET} PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

if(MSVC)
    set_target_properties(${XEUSCLING_TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)  # Internal string

    # Internal string
    set(cling_exports ?kEmptyCollection@valuePrinterInternal@cling@@3QEBDEB)

    # RTTI/C++ symbols
    set(cling_exports ${cling_exports} ??_7type_info@@6B@
        ?__type_info_root_node@@3U__type_info_node@@A
        ?nothrow@std@@3Unothrow_t@1@B
        ?_Facet_Register@std@@YAXPEAV_Facet_base@1@@Z
    )

    # Compiler added symbols for static variables. NOT for VStudio < 2015
    set(cling_exports ${cling_exports} _Init_thread_abort _Init_thread_epoch
        _Init_thread_footer _Init_thread_header _tls_index
    )

    # new/delete variants needed when linking to static msvc runtime (esp. Debug)
    set(cling_exports ${cling_exports}
        #??2@YAPEAX_KPEBDH@Z  ## not used in cling
        #??_U@YAPEAX_KPEBDH@Z ## not used in cling
        ??2@YAPEAX_K@Z
        ??3@YAXPEAX@Z
        ??_U@YAPEAX_K@Z
        ??_V@YAXPEAX@Z
    )

    # Most (if not all) of these MSVC decided are inlines that aren't exported
    # TODO: There are probably some missing. Why?
    set(cling_exports ${cling_exports} ?print@Decl@clang@@QEBAXAEAVraw_ostream@llvm@@I_N@Z
        ??6raw_ostream@llvm@@QEAAAEAV01@PEBX@Z
        ?decls_begin@DeclContext@clang@@QEBA?AVdecl_iterator@12@XZ
        ?errs@llvm@@YAAEAVraw_ostream@1@XZ
        ?grow_pod@SmallVectorBase@llvm@@IEAAXPEAX_K1@Z
        ?write@raw_ostream@llvm@@QEAAAEAV12@E@Z
        ?write@raw_ostream@llvm@@QEAAAEAV12@PEBD_K@Z
        ?castFromDeclContext@Decl@clang@@SAPEAV12@PEBVDeclContext@2@@Z
        ??1raw_ostream@llvm@@UEAA@XZ
        ??1raw_string_ostream@llvm@@UEAA@XZ
        ?flush_nonempty@raw_ostream@llvm@@AEAAXXZ
        ?getASTContext@Decl@clang@@QEBAAEAVASTContext@2@XZ
        ?handle@raw_ostream@llvm@@EEAAXXZ
        ?preferred_buffer_size@raw_ostream@llvm@@MEBA_KXZ
        ?write_impl@raw_string_ostream@llvm@@EEAAXPEBD_K@Z
        ?castToDeclContext@Decl@clang@@SAPEAVDeclContext@2@PEBV12@@Z
        ?classof@DeclContext@clang@@SA_NPEBVDecl@2@@Z
        ?setValueNoAlloc@internal@runtime@cling@@YAXPEAX00D_K@Z
        ?setValueNoAlloc@internal@runtime@cling@@YAXPEAX00DM@Z
        ?setValueNoAlloc@internal@runtime@cling@@YAXPEAX00DN@Z
        cling_runtime_internal_throwIfInvalidPointer
    )

    if($<CONFIG:Debug>)
        set(cling_exports ${cling_exports} ??$dyn_cast@VValueDecl@clang@@$$CBVDecl@2@@llvm@@YAPEBVValueDecl@clang@@PEBVDecl@2@@Z)
    endif()

    # List to '/EXPORT:sym0 /EXPORT:sym1 /EXPORT:sym2 ...'
    foreach(sym ${cling_exports})
        set(cling_link_str "${cling_link_str} /EXPORT:${sym}")
    endforeach(sym ${cling_exports})

    set_property(TARGET ${XEUSCLING_TARGET} APPEND_STRING PROPERTY LINK_FLAGS ${cling_link_str})
endif(MSVC)

target_link_libraries(${XEUSCLING_TARGET} ${LIBS})

################
# Installation #
################

# Install xeus-cling
install(TARGETS ${XEUSCLING_TARGET}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Configuration and data directories for jupyter and xeus-cling
set(XJUPYTER_DATA_DIR "share/jupyter"    CACHE STRING "Jupyter data directory")
set(XCPP_DATA_DIR     "share/xcpp"       CACHE STRING "xeus-cling data directory")
set(XCPP_CONF_DIR     "etc/xcpp"         CACHE STRING "xeus-cling configuration directory")

# Install xcpp headers
install(FILES ${XCPP_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xcpp)

# Install Jupyter kernelspecs
set(XCPP_KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
install(DIRECTORY ${XCPP_KERNELSPEC_DIR}
        DESTINATION ${XJUPYTER_DATA_DIR}
        PATTERN "*.in" EXCLUDE)

# Extra path for installing Jupyter kernelspec
if (XEXTRA_JUPYTER_DATA_DIR)
    install(DIRECTORY ${XCPP_KERNELSPEC_DIR}
            DESTINATION ${XEXTRA_JUPYTER_DATA_DIR}
            PATTERN "*.in" EXCLUDE)
endif(XEXTRA_JUPYTER_DATA_DIR)

# Install xeus-cling tag files
set(XCPP_TAGFILES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/xcpp/tagfiles)
install(DIRECTORY ${XCPP_TAGFILES_DIR}
        DESTINATION ${XCPP_DATA_DIR})

set(XCPP_TAGCONFS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/etc/xcpp/tags.d)
install(DIRECTORY ${XCPP_TAGCONFS_DIR}
        DESTINATION ${XCPP_CONF_DIR})

# Add definitions for the kernel to find tagfiles.
add_definitions(-DXCPP_TAGFILES_DIR="${CMAKE_INSTALL_PREFIX}/${XCPP_DATA_DIR}/tagfiles")
add_definitions(-DXCPP_TAGCONFS_DIR="${CMAKE_INSTALL_PREFIX}/${XCPP_CONF_DIR}/tags.d")
