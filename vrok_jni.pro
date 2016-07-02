
INCLUDEPATH += \
    /usr/lib/jvm/default-java/include \
    /usr/lib/jvm/default-java/include/linux

HEADERS += \
    jni_exposer.h \
    extern-jni.h \
    jbufferout.h

SOURCES += \
    jni_exposer.cpp \
    jbufferout.cpp

OTHER_FILES += \
    makefile_jni \
    makefile
