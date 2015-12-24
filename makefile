OBJECTS= \
    threadpool.o \
    queue.o \
    buffer.o \
    ffmpeg.o \
    bgpoint.o \
    debug.o \
    driver.o \
    audioout.o \
    effect.o \
    preamp.o \
    player.o \
    fir.o \
    tapdistortion.o \
    componentmanager.o \
    component.o \
    eq.o \
    shibatch/equ.o \
    shibatch/ooura_fft.o \
    alsa.o \
    notify.o \
    vumeter.o \
    common.o 
PY_OBJECTS= \
    py_exposer.o

CC=gcc
CXX=g++
INCLUDE= -I/usr/include/python3.5m
CFLAGS= -g -O2 -fPIC  -DUSE_OOURA $(INCLUDE)
CXXFLAGS= -g -O2 -fPIC -std=c++11 -DUSE_OOURA $(INCLUDE)
LDFLAGS= -shared -lavformat -lavcodec -lavutil -lao -lasound -lboost_python3 -L/usr/lib/python3.5/config-3.5m/ -lpython3.5m
LIBNAME_PY = vrok.so

$(LIBNAME_PY): $(OBJECTS)  $(PY_OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(PY_OBJECTS)  -o $@
	
all: $(EXECUTABLE)
	