#-------------------------------------------------
#
# Project created by QtCreator 2014-12-28T16:53:17
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = threadpool
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -O3 -std=c++11 -ffast-math -msse4.2

SOURCES += main.cpp \
    threadpool.cpp \
    queue.cpp \
    buffer.cpp \
    ffmpeg.cpp \
    bgpoint.cpp \
    test1.cpp \
    debug.cpp \
    test2.cpp \
    driver.cpp \
    audioout.cpp \
    effect.cpp \
    preamp.cpp \
    player.cpp \
    fir.cpp \
    tapdistortion.cpp \
    componentmanager.cpp \
    component.cpp

HEADERS += \
    threadpool.h \
    queue.h \
    buffer.h \
    ffmpeg.h \
    bgpoint.h \
    bufferconfig.h \
    test1.h \
    debug.h \
    test2.h \
    stopwatch.h \
    ringbuffer.h \
    driver.h \
    audioout.h \
    effect.h \
    preamp.h \
    player.h \
    decoder.h \
    resource.h \
    delaybuffer.h \
    fir.h \
    tapdistortion.h \
    componentmanager.h \
    component.h \
    common.h

LIBS        += -lavformat -lavcodec -lavutil -lao

CONFIG(debug)
{
    DEFINES += DEBUG
}