#include "player.h"
#include "audiotrack.h"
#include "preamp.h"
#include "fir.h"
#include <jni.h>

extern JavaVM* javaVM;

Vrok::Player *pl;
Vrok::DriverAudioTrack *out;
Vrok::EffectFIR *pre;
Vrok::EffectFIR *pre1;

static jobject hookObject=NULL;
static jmethodID hookMethod=NULL;
static jclass hookClass=NULL;

static Vrok::Player *plx=NULL;

void nextTrackCallback();

extern "C"
{
    Vrok::Resource *CreateResource(const char *filename)
    {
        Vrok::Resource *res=new Vrok::Resource();
        res->_filename = std::string(filename);
        return res;
    }
    void DestroyResource(Vrok::Resource *resource)
    {
        delete resource;
    }
    void RegisterSource(BufferGraph::Point *parent, BufferGraph::Point *source)
    {
        parent->RegisterSource(source);
    }
    void RegisterSink(BufferGraph::Point *parent, BufferGraph::Point *sink)
    {
        parent->RegisterSource(sink);
    }

    Vrok::Player *CreatePlayer()
    {
        pre->RegisterSource(pl);
        pre->RegisterSink(out);
        pl->RegisterSink(pre);
    	out->RegisterSource(pre);

    	out->Preallocate();
        pl->Preallocate();
        pre->Preallocate();
//        pre1->Preallocate();

        pre->CreateThread();
  //      pre1->CreateThread();
        out->CreateThread();
        pl->CreateThread();

        pl->SetNextTrackCallback(nextTrackCallback);
        return pl;
    }
    void JoinPlayer()
    {
        pl->JoinThread();
        pre->JoinThread();
       // pre1->JoinThread();
        out->JoinThread();
    }
    void SubmitForPlayback(Vrok::Resource *resource)
    {
        pl->SubmitForPlayback(resource);
    }
    void SubmitForPlaybackNow(Vrok::Resource *resource)
    {
        pl->SubmitForPlaybackNow(resource);
    }
}


void nextTrackCallback() {

    if (hookObject) {
        JNIEnv *g_env;
        int getEnvStat = javaVM->GetEnv((void **)&g_env, JNI_VERSION_1_6);
        if (getEnvStat == JNI_EDETACHED) {
            if (javaVM->AttachCurrentThread(&g_env, NULL) != 0) {
                DBG("Failed to attach");
            }
        } else if (getEnvStat == JNI_OK) {
            //
        } else if (getEnvStat == JNI_EVERSION) {
            DBG("GetEnv: version not supported");
        }
        jstring str = (jstring) g_env->CallNonvirtualObjectMethod(hookObject, hookClass, hookMethod );
        //strcpy(url, g_env->GetStringUTFChars( str , NULL ) );

        plx->SubmitForPlaybackNow(CreateResource(g_env->GetStringUTFChars( str , NULL )));

        if (g_env->ExceptionCheck()) {
            g_env->ExceptionDescribe();
        }

        javaVM->DetachCurrentThread();  
    } else  {
        DBG("Callback interface not initialized");
    }
}
extern "C" {
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    javaVM = vm;
    pl = new Vrok::Player;
    out = new Vrok::DriverAudioTrack;
    pre = new Vrok::EffectFIR;
    pre1 = new Vrok::EffectFIR;
    
    return JNI_VERSION_1_6;
}
JNIEXPORT void JNICALL
JNI_OnUnLoad(JavaVM *vm, void *reserved) { 
    delete pl;
    delete out;
    delete pre;
    delete pre1;
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_hookCallback( JNIEnv* env,
                                             jobject thiz)
{
   
    hookObject = env->NewGlobalRef(thiz);

    // save refs for callback
    hookClass = (jclass) env->NewGlobalRef( (jobject) env->GetObjectClass(hookObject) );
    if (hookClass == NULL) {
        DBG("Failed to find class");
    }

    hookMethod = env->GetMethodID(hookClass, "nextTrack", "()Ljava/lang/String;");
   // hookMethod = env->GetStaticMethodID(hookClass, "nextTrack", "()Ljava/lang/String;");
    if (hookMethod == NULL) {
        DBG("Unable to get method ref");
    }
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_initialize( JNIEnv* env,
                                   jobject thiz,
                                   jstring settingsPath )
{
    plx = CreatePlayer();
	
}
JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_unhookCallback( JNIEnv* env,
                                                   jobject thiz)
{
    env->DeleteGlobalRef(hookObject);
    env->DeleteGlobalRef((jobject)hookClass);
    hookMethod = NULL;
    hookClass =NULL;
    hookObject =NULL;
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_open( JNIEnv* env,
                                             jobject thiz,
                                             jstring path )
{
	const char *szPath = env->GetStringUTFChars( path , NULL );
	plx->SubmitForPlaybackNow(CreateResource(szPath));
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_play( JNIEnv* env,
                                         jobject thiz
                                       )
{
    
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_pause( JNIEnv* env,
                                          jobject thiz
                                        )
{
}

// Position is set in normalized range, 0..1
JNIEXPORT float JNICALL
Java_com_mx_vrok_VrokService_getPosition( JNIEnv* env,
                                                jobject thiz)
{
    return 0;
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_setPosition( JNIEnv* env,
                                                jobject thiz,
                                                float pos )
{
    
}

JNIEXPORT bool JNICALL
Java_com_mx_vrok_VrokService_getEqualizerAutoPreamp( JNIEnv* env,
                                                           jobject thiz
                                                         )
{
    
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_setEqualizerAutoPreamp( JNIEnv* env,
                                                           jobject thiz,
                                                           bool value
                                                         )
{
    
}

JNIEXPORT float JNICALL
Java_com_mx_vrok_VrokService_getEqualizerPreamp( JNIEnv* env,
                                                       jobject thiz
                                                     )
{
    return 0;
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_setEqualizerPreamp( JNIEnv* env,
                                                       jobject thiz,
                                                       float value
                                                     )
{
    
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_setEqualizerBand( JNIEnv* env,
                                                     jobject thiz,
                                                     int band, 
                                                     float value
                                                   )
{
    
}

JNIEXPORT float JNICALL
Java_com_mx_vrok_VrokService_getEqualizerBand( JNIEnv* env,
                                                     jobject thiz,
                                                     int band
                                                   )
{
    return 0;
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_shutdown( JNIEnv* env,
                                                     jobject thiz
                                                   )
{
    
}

}


