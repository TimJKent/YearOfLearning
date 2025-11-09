cmake_minimum_required(VERSION 3.10)
project(WebcamRendering)

add_subdirectory(SOGLR)
include_directories(SOGLR/include)
    if(NOT WIN32)
        message(FATAL_ERROR "WebcamRendering can only be built on Windows.")
    endif()
add_executable(WebcamRendering src/main.cpp)
target_compile_features(WebcamRendering PRIVATE cxx_std_17)
target_link_libraries(WebcamRendering PRIVATE 
    SOGLR
    mf
    mfplat
    mfreadwrite
    mfuuid
    ole32
    oleaut32
    strmiids
)