
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

SOURCES += \
    com/mx/vrok/VrokServices.java \
    com/mx/vrok/EventCallback.java \
    TestEventCallback.java \
    test.java

OTHER_FILES += \
    makefile_jni \
    makefile
