cmake_minimum_required (VERSION 3.8)

project (vrok)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(ASAN_LIBRARIES asan)
set(FFMPEG_LIBRARIES avformat avcodec avutil)
set(ALSA_LIBRARIES asound)
set(SYSTEM_LIBRARIES -pthread -lm)

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
    "alsa.*"
    "notify.*"
    "notifier_impl.*"
    "vumeter.*"
    "resampler.*"
    "common.*"
    "notifier_impl.*"
    "simple_main.cpp"
)


add_definitions(-DUSE_OOURA)



add_library(vrok SHARED ${COMMON_SRC})
#add_executable(vrok ${COMMON_SRC})

set_target_properties(vrok PROPERTIES PREFIX "" )
target_link_libraries(vrok
    #${ASAN_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    ${SYSTEM_LIBRARIES}
    ${ALSA_LIBRARIES})
