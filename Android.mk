
LOCAL_PATH := $(call my-dir)
LOCAL_MODULE_FILENAME := libvrok
LOCAL_MODULE    := vrok
LOCAL_CFLAGS := -std=c++11 -ffast-math -O3 -mfpu=neon -funsafe-math-optimizations
LOCAL_SRC_FILES := extern-jni.cpp \
		threadpool.cpp \
		queue.cpp \
		buffer.cpp \
		ffmpeg.cpp \
		bgpoint.cpp \
		test1.cpp \
		debug.cpp \
		test2.cpp \
		driver.cpp \
		audiotrack.cpp \
		effect.cpp \
		preamp.cpp \
		player.cpp \
		fir.cpp \
		tapdistortion.cpp \
		componentmanager.cpp \
		component.cpp

LOCAL_LDLIBS := -llog -ljnigraphics -lz -landroid 

LOCAL_SHARED_LIBRARIES := libavformat libavcodec libswscale libavutil libswresample

 

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, /media/ENT/opt/android-ndk-r9c/sources/)
$(call import-module,ffmpeg-2.6.1/android/arm/)