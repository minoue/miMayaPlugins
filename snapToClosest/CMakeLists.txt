cmake_minimum_required(VERSION 3.8)
project(snapToClosest CXX)

set(SOURCE_FILES
        src/pluginMain.cpp
        src/snapToClosest.h
        src/snapToClosest.cpp
        )


if(NOT MAYA_ROOT_DIR)
    message(FATAL_ERROR "MAYA_ROOT_DIR must be set!")
else()
    message(STATUS "Using MAYA_ROOT_DIR ${MAYA_ROOT_DIR}")
endif()

# https://www.guyrutenberg.com/2014/01/05/enabling-c11-c0x-in-cmake/
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mtune=native -march=native")

if (COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(MAYA_VERSION 2018 CACHE STRING "Maya version")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Default build type")


if (WIN32)
    set(MAYA_INCLUDE_DIR ${MAYA_ROOT_DIR}/include)
    set(MAYA_LIB_DIR ${MAYA_ROOT_DIR}/lib)
    set(MAYA_COMPILE_DEFINITIONS "REQUIRE_IOSTREAM;_BOOL;NT_PLUGIN")
    set(MAYA_PLUGIN_EXTENSION ".mll")
    set(MAYA_TARGET_TYPE RUNTIME)
    set(INSTALL_DIR ../plug-ins/win_${MAYA_VERSION})
elseif (APPLE)
    set(MAYA_INCLUDE_DIR ${MAYA_ROOT_DIR}/include)
    set(MAYA_LIB_DIR ${MAYA_ROOT_DIR}/lib)
    set(MAYA_COMPILE_DEFINITIONS "REQUIRE_IOSTREAM;_BOOL;OSMac_")
    set(MAYA_PLUGIN_EXTENSION ".bundle")
    set(MAYA_TARGET_TYPE LIBRARY)
    set(CMAKE_MACOSX_RPATH ON)
    set(INSTALL_DIR ../plug-ins/macOS_${MAYA_VERSION})
else ()
    set(MAYA_INCLUDE_DIR ${MAYA_ROOT_DIR}/include)
    set(MAYA_LIB_DIR ${MAYA_ROOT_DIR}/lib)
    set(MAYA_COMPILE_DEFINITIONS "REQUIRE_IOSTREAM;_BOOL;LINUX")
    set(MAYA_PLUGIN_EXTENSION ".so")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(MAYA_TARGET_TYPE LIBRARY)
    set(INSTALL_DIR ../plug-ins/linux_${MAYA_VERSION})
endif()


include_directories(${MAYA_INCLUDE_DIR})
link_directories(${MAYA_LIB_DIR})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
        OpenMaya
        OpenMayaAnim
        OpenMayaFX
        OpenMayaRender 
        OpenMayaUI
        Foundation
        )

if (WIN32)
set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/export:initializePlugin /export:uninitializePlugin"
        )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
        COMPILE_DEFINITIONS "${MAYA_COMPILE_DEFINITIONS}"
        PREFIX ""
        SUFFIX ${MAYA_PLUGIN_EXTENSION}
        )

install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION ${INSTALL_DIR})
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
