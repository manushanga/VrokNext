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
    player.o \
    fir.o \
    tapdistortion.o \
    componentmanager.o \
    component.o \
    eq.o \
    shibatch/equ.o \
    shibatch/ooura_fft.o \
    util/mutil.o \
    util/sharedmem.o \
    alsa.o \
    notify.o \
    notifier_impl.o \
    vumeter.o \
    jbufferout.o \
    common.o 
PY_OBJECTS= \
    py_exposer.o

CC=gcc
CXX=g++
INCLUDE= -I/usr/include/python3.8 -I/usr/include/x86_64-linux-gnu/python3.8
CFLAGS= -g -O2 -fPIC  -DUSE_OOURA $(INCLUDE)
CXXFLAGS= -g -O2 -fPIC -std=c++11 -DUSE_OOURA $(INCLUDE) 
LDFLAGS= -shared -lavformat -lavcodec -lavutil -lao -lasound -lboost_python38 -lboost_numpy38 -lm -lpthread 
LIBNAME_PY = vrok.so

$(LIBNAME_PY): $(OBJECTS)  $(PY_OBJECTS)
	$(CC) $(OBJECTS) $(PY_OBJECTS)  -o $@  $(LDFLAGS)
	
all: $(EXECUTABLE)

clean:
	rm *.o 
	rm vrok.so
