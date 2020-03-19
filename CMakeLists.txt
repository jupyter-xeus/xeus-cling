####################################################################################
# Copyright (c) 2016, Johan Mabille, Loic Gouarin, Sylvain Corlay, Wolf Vollprecht #
# Copyright (c) 2016, QuantStack                                                   #
#                                                                                  #
# Distributed under the terms of the BSD 3-Clause License.                         #
#                                                                                  #
# The full license is in the file LICENSE, distributed with this software.         #
####################################################################################

cmake_minimum_required(VERSION 3.4.3)
project(xeus-cling)

set(XEUS_CLING_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Versionning
# ===========

file(STRINGS "${XEUS_CLING_INCLUDE_DIR}/xeus-cling/xeus_cling_config.hpp" xeus_cling_version_defines
     REGEX "#define XEUS_CLING_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xeus_cling_version_defines})
    if(ver MATCHES "#define XEUS_CLING_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XEUS_CLING_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
    ${XEUS_CLING_VERSION_MAJOR}.${XEUS_CLING_VERSION_MINOR}.${XEUS_CLING_VERSION_PATCH})
message(STATUS "Building xeus-cling v${${PROJECT_NAME}_VERSION}")

# Configuration
# =============

include(GNUInstallDirs)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xcpp11/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xcpp11/kernel.json"
)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xcpp14/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xcpp14/kernel.json"
)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xcpp17/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xcpp17/kernel.json"
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

include_directories(${LLVM_MAIN_INCLUDE_DIR})
link_directories(${LLVM_LIBRARY_DIR})
add_definitions(-DLLVM_DIR="${LLVM_BINARY_DIR}")

################
# Dependencies #
################

set(xeus_REQUIRED_VERSION 0.21.1)
set(cppzmq_REQUIRED_VERSION 4.3.0)

find_package(xeus ${xeus_REQUIRED_VERSION} REQUIRED)
find_package(cppzmq ${cppzmq_REQUIRED_VERSION} REQUIRED)
find_package(pugixml REQUIRED)

find_package(Clang REQUIRED)
find_package(Cling REQUIRED)
find_package(cxxopts REQUIRED)

#########
# flags #
#########

include(CheckCXXCompilerFlag)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the build type, e.g. Release or Debug." FORCE)
endif()

if(MSVC)
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

# xeus-cling sources
set(XEUS_CLING_SRC
    src/xinput.hpp
    src/xinput.cpp
    src/xinterpreter.cpp
    src/xdemangle.hpp
    src/xoptions.cpp
    src/xparser.cpp
    src/xparser.hpp
    src/xpaths.cpp
    src/xpaths.hpp
    src/xholder_cling.cpp
    src/xmagics/executable.cpp
    src/xmagics/executable.hpp
    src/xmagics/execution.cpp
    src/xmagics/execution.hpp
    src/xmagics/os.cpp
    src/xmagics/os.hpp
    src/xmime_internal.hpp
)

# xeus-cling headers
set(XEUS_CLING_HEADERS
    include/xeus-cling/xbuffer.hpp
    include/xeus-cling/xeus_cling_config.hpp
    include/xeus-cling/xholder_cling.hpp
    include/xeus-cling/xinterpreter.hpp
    include/xeus-cling/xmagics.hpp
    include/xeus-cling/xmanager.hpp
    include/xeus-cling/xoptions.hpp
    include/xeus-cling/xpreamble.hpp
)

# xcpp sources
set(XCPP_SRC
    src/main.cpp
    src/xpaths.hpp
    src/xpaths.cpp
)

# xcpp headers (needed at runtime by the C++ kernel)
set(XCPP_HEADERS
    include/xcpp/xmime.hpp
    include/xcpp/xdisplay.hpp
)

# xeus-cling is the target for the library
add_library(xeus-cling SHARED ${XEUS_CLING_SRC} ${XEUS_CLING_HEADERS})

# xcpp is the target for the kernel executable
add_executable(xcpp ${XCPP_SRC})
set_target_properties(xcpp PROPERTIES ENABLE_EXPORTS 1)
target_link_libraries(xcpp PRIVATE xeus-cling)

set(LLVM_NO_DEAD_STRIP 1)

if (APPLE)
    set_target_properties(xcpp xeus-cling PROPERTIES
        MACOSX_RPATH ON
    )
else()
    set_target_properties(xcpp xeus-cling PROPERTIES
        BUILD_WITH_INSTALL_RPATH 1
        SKIP_BUILD_RPATH FALSE
    )
endif()

set_target_properties(xcpp xeus-cling PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

target_include_directories(xeus-cling
                           PUBLIC
                           $<BUILD_INTERFACE:${XEUS_CLING_INCLUDE_DIR}>
                           $<INSTALL_INTERFACE:include>)
target_link_libraries(xeus-cling PUBLIC clingInterpreter clingMetaProcessor clingUtils xeus pugixml cxxopts::cxxopts)

set_target_properties(xeus-cling PROPERTIES
                      PUBLIC_HEADER "${XEUS_CLING_HEADERS}"
                      COMPILE_DEFINITIONS "XEUS_CLING_EXPORTS"
                      PREFIX ""
                      VERSION ${${PROJECT_NAME}_VERSION}
                      SOVERSION ${XEUS_CLING_VERSION_MAJOR}
                      OUTPUT_NAME "libxeus-cling")

if(MSVC)
    set_target_properties(xeus-cling PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS 1)  # Internal string

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
        ??3@YAXPEAX_K@Z
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
        ?setValueNoAlloc@internal@runtime@cling@@YAXPEAX00D@Z
        ?setValueNoAlloc@internal@runtime@cling@@YAXPEAX00D_K@Z
        ?setValueNoAlloc@internal@runtime@cling@@YAXPEAX00DM@Z
        ?setValueNoAlloc@internal@runtime@cling@@YAXPEAX00DN@Z
        ?setValueWithAlloc@internal@runtime@cling@@YAPEAXPEAX00D@Z
        ?setValueNoAlloc@internal@runtime@cling@@YAXPEAX00DPEBX@Z
        cling_runtime_internal_throwIfInvalidPointer
    )

    set(cling_exports ${cling_exports}
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEAPEBD@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEAPEBX@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBC@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBD@Z 
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBE@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBF@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBG@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBH@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBI@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBJ@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBK@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBM@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBN@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBO@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBQEBD@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBV23@@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBV?$$basic_string@_SU?$$char_traits@_S@std@@V?$$allocator@_S@2@@3@@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBV?$$basic_string@_UU?$$char_traits@_U@std@@V?$$allocator@_U@2@@3@@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBV?$$basic_string@_WU?$$char_traits@_W@std@@V?$$allocator@_W@2@@3@@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBVValue@1@@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEBX@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEB_J@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEB_K@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEB_N@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEB_S@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEB_U@Z
        ?printValue@cling@@YA?AV?$$basic_string@DU?$$char_traits@D@std@@V?$$allocator@D@2@@std@@PEB_W@Z
    )

    if($<CONFIG:Debug>)
        set(cling_exports ${cling_exports} ??$dyn_cast@VValueDecl@clang@@$$CBVDecl@2@@llvm@@YAPEBVValueDecl@clang@@PEBVDecl@2@@Z)
    endif()

    # List to '/EXPORT:sym0 /EXPORT:sym1 /EXPORT:sym2 ...'
    foreach(sym ${cling_exports})
        set(cling_link_str "${cling_link_str} /EXPORT:${sym}")
    endforeach(sym ${cling_exports})

    set_property(TARGET xeus-cling APPEND_STRING PROPERTY LINK_FLAGS ${cling_link_str})
endif(MSVC)

#########
# Tests #
#########

OPTION(BUILD_TESTS "xeus-cling test suite" OFF)
OPTION(DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)

if(DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    set(BUILD_TESTS ON)
endif()

if(BUILD_TESTS)
    add_subdirectory(test)
endif()

################
# Installation #
################

include(CMakePackageConfigHelpers)

set(XEUS_CLING_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xeus-clingConfig.cmake")

# Install xeus-cling and xcpp
install(TARGETS xeus-cling
        EXPORT ${PROJECT_NAME}-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus-cling)
install(TARGETS xcpp
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Configuration and data directories for jupyter and xeus-cling
set(XJUPYTER_DATA_DIR   "share/jupyter"    CACHE STRING "Jupyter data directory")
set(XEUS_CLING_DATA_DIR "share/xeus-cling" CACHE STRING "xeus-cling data directory")
set(XEUS_CLING_CONF_DIR "etc/xeus-cling"   CACHE STRING "xeus-cling configuration directory")

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
set(XCPP_TAGFILES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/xeus-cling/tagfiles)
install(DIRECTORY ${XCPP_TAGFILES_DIR}
        DESTINATION ${XEUS_CLING_DATA_DIR})

set(XCPP_TAGCONFS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/etc/xeus-cling/tags.d)
install(DIRECTORY ${XCPP_TAGCONFS_DIR}
        DESTINATION ${XEUS_CLING_CONF_DIR})

# Add definitions for the kernel to find tagfiles.
add_definitions(-DXCPP_TAGFILES_DIR="${XEUS_CLING_DATA_DIR}/tagfiles")
add_definitions(-DXCPP_TAGCONFS_DIR="${XEUS_CLING_CONF_DIR}/tags.d")

# Makes the project importable from the build directory
export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

# Configure 'xeus-clingConfig.cmake' for a build tree
set(XEUS_CLING_CONFIG_CODE "####### Expanded from \@XEUS_CLING_CONFIG_CODE\@ #######\n")
set(XEUS_CLING_CONFIG_CODE "${XEUS_CLING_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XEUS_CLING_CONFIG_CODE "${XEUS_CLING_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xeus-clingConfig.cmake' for an install tree
set(XEUS_CLING_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XEUS_CLING_CMAKECONFIG_INSTALL_DIR})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${XEUS_CLING_CMAKECONFIG_INSTALL_DIR})
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${XEUS_CLING_CMAKECONFIG_INSTALL_DIR})
