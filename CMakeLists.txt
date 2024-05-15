cmake_minimum_required (VERSION 3.8)

project (vrok)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS -g)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(ASAN_LIBRARIES asan)
set(FFMPEG_LIBRARIES avformat avcodec avutil)
set(ALSA_LIBRARIES asound)
set(SYSTEM_LIBRARIES -pthread -lm)


find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

add_definitions(-DDEBUG)
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
    "resampler/resampler.*"
    "util/mutil.*"
    "util/fastmath.*"
    "util/sharedmem.*"
    "pulse.*"
    "notify.*"
    "notifier_impl.*"
    "vumeter.*"
    "resampler.*"
    "common.*"
    "notifier_impl.*"
    "metadata.*"
    "simple_main.cpp"
)


add_definitions(-DUSE_OOURA)



#add_library(vrok SHARED ${COMMON_SRC})
add_executable(vrok ${COMMON_SRC})
target_include_directories(vrok PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR})
set_target_properties(vrok PROPERTIES PREFIX "" )
target_link_libraries(vrok
    -lpulse-simple
    #${ASAN_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    ${SYSTEM_LIBRARIES}
    ${ALSA_LIBRARIES})
