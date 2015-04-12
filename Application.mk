#APP_ABI := all


include $(CLEAR_VARS)
APP_STL := gnustl_static

APP_OPTIM := release
APP_PLATFORM := android-15
APP_CPPFLAGS += -frtti 
APP_CPPFLAGS += -fexceptions
APP_ABI := armeabi-v7a
