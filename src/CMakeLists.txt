############################################################################
# Copyright (c) 2016, Johan Mabille and Sylvain Corlay                     #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.4.3)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

set(XEUSCLING_SRC
    main.cpp
    xcpp_interpreter.cpp
    xcpp_interpreter.hpp
)

# Target and link
#################

set(LIBS ${xeus_LIBRARY} clingInterpreter clingMetaProcessor clingUtils)

# TODO: remove this workaround when Cling package is fixed
if(MSVC)
    set(LIBS ${LIBS} clangAnalysis clangEdit LLVMCoverage Version)
endif()

set(XEUSCLING_TARGET xeus-cling)
add_executable(${XEUSCLING_TARGET} ${XEUSCLING_SRC})
target_link_libraries(${XEUSCLING_TARGET} ${LIBS})
