cmake_minimum_required(VERSION 2.8)
project(symmetryTable)

set(SOURCE_FILES
    src/symmetryTable.cpp
    src/symmetryTable.h
    src/pluginMain.cpp
    )

if(NOT MAYA_ROOT_DIR)
    message(FATAL_ERROR "MAYA_ROOT_DIR must be set!")
else()
    message(STATUS "Using MAYA_ROOT_DIR ${MAYA_ROOT_DIR}")
endif()

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mtune=native -march=native")
endif()

if (COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(MAYA_VERSION 2018 CACHE STRING "Maya version")
set(CMAKE_BUILD_TYPE Release CACHE STRING "Default build type")


if (WIN32)
    set(INCLUDE_DIR ${MAYA_ROOT_DIR}/include)
    set(LIBRARY_DIR ${MAYA_ROOT_DIR}/lib)
    set(MAYA_COMPILE_DEFINITIONS "REQUIRE_IOSTREAM;_BOOL;NT_PLUGIN")
    set(MAYA_PLUGIN_EXTENSION ".mll")
    set(MAYA_TARGET_TYPE RUNTIME)
    set(INSTALL_DIR ../plug-ins/win_${MAYA_VERSION})
elseif (APPLE)
    set(INCLUDE_DIR ${MAYA_ROOT_DIR}/include)
    set(LIBRARY_DIR ${MAYA_ROOT_DIR}/Maya.app/Contents/MacOS)
    set(MAYA_COMPILE_DEFINITIONS "REQUIRE_IOSTREAM;_BOOL;OSMac_")
    set(MAYA_PLUGIN_EXTENSION ".bundle")
    set(MAYA_TARGET_TYPE LIBRARY)
    set(CMAKE_MACOSX_RPATH ON)
    set(INSTALL_DIR ../plug-ins/macOS_${MAYA_VERSION})
else ()
    set(INCLUDE_DIR "")
    set(LIBRARY_DIR "")
    set(MAYA_COMPILE_DEFINITIONS "REQUIRE_IOSTREAM;_BOOL;LINUX")
    set(MAYA_PLUGIN_EXTENSION ".so")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(MAYA_TARGET_TYPE LIBRARY)
    set(INSTALL_DIR ../plug-ins/linux_${MAYA_VERSION})
endif()


include_directories(${INCLUDE_DIR})
link_directories(${LIBRARY_DIR})
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
