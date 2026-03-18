cmake_minimum_required(VERSION 3.14)
project(deepstream_gst_app CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
    gstreamer-1.0
    gstreamer-video-1.0
)
include_directories(
    ${GST_INCLUDE_DIRS}
    /opt/nvidia/deepstream/deepstream-8.0/includes
    /opt/nvidia/deepstream/deepstream-8.0/sources/includes
    ${CMAKE_SOURCE_DIR}/third_party   
)
link_directories(
    /opt/nvidia/deepstream/deepstream-8.0/lib
)
set(APP_SOURCES
    src/main.cpp
    src/roi.cpp
    src/dwell.cpp
    src/fps_counter.cpp
    src/overlay.cpp
    src/probe.cpp
    src/pipeline.cpp
)
add_executable(deepstream_gst_app ${APP_SOURCES})
target_compile_options(deepstream_gst_app PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-g -O0>
)
target_link_libraries(deepstream_gst_app
    PRIVATE
        ${GST_LIBRARIES}
        nvds_meta
        nvds_infer
        nvdsgst_meta
        nvds_nvmultiobjecttracker
        pthread
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
