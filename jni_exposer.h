#pragma once

#include <jni.h>

extern JavaVM* g_VM;

void CreateContext();
void DeleteContext();
void CreateThreads();
void JoinThreads();

extern "C" 
{
    
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    g_VM = vm;
    CreateContext();
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
JNI_OnUnLoad(JavaVM *vm, void *reserved) 
{ 
    DeleteContext();
}

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_open
  (JNIEnv *, jobject, jstring);

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_openSingleThread
  (JNIEnv *, jobject, jstring, int, bool);

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_setSamplerate
  (JNIEnv *, jobject, int, int);
  
JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_close
  (JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_createThreads
  (JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_joinThreads
  (JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_setupCallbacks
  (JNIEnv *, jobject , int, jobject);

}
