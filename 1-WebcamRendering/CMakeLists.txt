cmake_minimum_required(VERSION 3.10)
project(WebcamRendering)

add_subdirectory(SOGLR)
add_subdirectory(CPCam)

include_directories(SOGLR/include)
include_directories(CPCam/include)


add_executable(WebcamRendering src/main.cpp)
target_compile_features(WebcamRendering PRIVATE cxx_std_17)

set(SYSTEM_LIBRARIES "")

if(WIN32)
    set(SYSTEM_LIBRARIES
        mf
        mfplat
        mfreadwrite
        mfuuid
        ole32
        oleaut32
        strmiids
    )
elseif(UNIX)
    set(SYSTEM_LIBRARIES
        GL
        pthread
    )
endif()


target_link_libraries(WebcamRendering PRIVATE 
    SOGLR
    CPCam
    ${SYSTEM_LIBRARIES}
)