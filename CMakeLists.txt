cmake_minimum_required (VERSION 3.8)

project (vrok)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil)

if(APPLE)
    pkg_check_modules(AO REQUIRED IMPORTED_TARGET ao)
    set(AUDIO_SOURCE audioout.cpp)
    set(AUDIO_LIBRARY PkgConfig::AO)
else()
    pkg_check_modules(PULSE REQUIRED IMPORTED_TARGET libpulse-simple)
    set(AUDIO_SOURCE pulse.cpp)
    set(AUDIO_LIBRARY PkgConfig::PULSE)
endif()

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
    "notify.*"
    "notifier_impl.*"
    "vumeter.*"
    "resampler.*"
    "common.*"
    "notifier_impl.*"
    "metadata.*"
)

add_library(vrok SHARED ${COMMON_SRC} ${AUDIO_SOURCE})
target_compile_definitions(vrok PRIVATE USE_OOURA $<$<CONFIG:Debug>:DEBUG>)
target_link_libraries(vrok PRIVATE PkgConfig::FFMPEG Threads::Threads)
# The selected driver's public header includes its backend's headers.
target_link_libraries(vrok PUBLIC ${AUDIO_LIBRARY})

add_executable(simple_main simple_main.cpp)
target_link_libraries(simple_main PRIVATE vrok)
