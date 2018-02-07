cmake_minimum_required (VERSION 3.8)

project (vrok)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(Boost_NO_SYSTEM_PATHS  TRUE)

if (NOT BOOST_ROOT)
    message(FATAL_ERROR "BOOST_DIR not defined")
endif()

set(BOOST_INCLUDEDIR ${BOOST_ROOT})
set(Boost_INCLUDE_DIR ${BOOST_ROOT})
set(Boost_LIBRARY_DIR "${BOOST_ROOT}/stage/lib")
set(PYTHON_EXECUTABLE "/usr/bin/python3")

set(BOOST_LIBRARIES boost_python3 boost_numpy3)
set(FFMPEG_LIBRARIES avformat avcodec avutil)
set(SAMPLERATE_LIBRARIES samplerate)
set(ALSA_LIBRARIES asound)

find_package(PythonDev)


file(GLOB COMMON_SRC
    "threadpool.*"
    "queue.*"
    "buffer.*"
    "ffmpeg.*"
    "bgpoint.*"
    "debug.*"
    "driver.*"
    "effect.*"
    "player.*"
    "componentmanager.*"
    "component.*"
    "eq.*"
    "shibatch/equ.*"
    "shibatch/ooura_fft.*"
    "util/mutil.*"
    "util/fastmath.*"
    "alsa.*"
    "notify.*"
    "notifier_impl.*"
    "vumeter.*"
    "resampler.*"
    "common.*"
)


file(GLOB JAVA_SRC
    "jbufferout.*"
    "jni_exposer.*"
)
file(GLOB PYTHON_SRC
    "jbufferout.*"
    "py_exposer.*"
)

add_definitions(-DUSE_OOURA)

if (BUILD_MODE STREQUAL "PYTHON")
    include_directories(${Boost_INCLUDE_DIR})
    include_directories(${PYTHON_INCLUDE_DIR})

    link_directories(${Boost_LIBRARY_DIR})

    add_library(vrok SHARED ${COMMON_SRC} ${PYTHON_SRC})
    set_target_properties(vrok PROPERTIES PREFIX "" )
    target_link_libraries(vrok
        ${FFMPEG_LIBRARIES}
        ${PYTHON_LIBRARIES}
        ${BOOST_LIBRARIES}
        ${SAMPLERATE_LIBRARIES}
        ${ALSA_LIBRARIES})
else()
    message(FATAL_ERROR "BUILD_MODE=${BUILD_MODE} not supported")
endif()

