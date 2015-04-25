#include "player.h"
#include "audiotrack.h"
#include "preamp.h"
#include "fir.h"
#include "eq.h"
#include "threadpool.h"
#include "componentmanager.h"
#include <jni.h>

extern JavaVM* javaVM;

Vrok::Player *pl;
Vrok::DriverAudioTrack *out;
Vrok::EffectSSEQ *pSSEQ;
Vrok::EffectFIR *pFIR;
Vrok::Component *current=nullptr;

ThreadPool *pool;

static jobject hookObject=NULL;
static jmethodID hookMethod=NULL;
static jclass hookClass=NULL;

static Vrok::Player *plx=NULL;

void NextTrackCallback(void *user);

extern "C"
{
    void CreateContext()
    {
        
        pl = new Vrok::Player;
        out = new Vrok::DriverAudioTrack;
        pSSEQ = new Vrok::EffectSSEQ;
        pFIR = new Vrok::EffectFIR;
        pool = new ThreadPool(4);
  
    }
    
    void SetCurrentComponent(const char *name)
    {
        current = Vrok::ComponentManager::GetSingleton()->GetComponent(std::string(name));
    }
    
    void SetPropertyFloat(const char *name, float val)
    {
        Vrok::PropertyBase *p = Vrok::ComponentManager::GetSingleton()->GetProperty(
            current,
            std::string(name)
        );
        
        Vrok::ComponentManager::GetSingleton()->SetProperty(current, p, &val);
        
    }
    
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
        DBG("player");
        pFIR->RegisterSource(pl);
        pFIR->RegisterSink(pSSEQ);

        pSSEQ->RegisterSource(pFIR);
        pSSEQ->RegisterSink(out);

        pl->RegisterSink(pFIR);

        out->RegisterSource(pSSEQ);
        
        DBG("Reg");

        out->Preallocate();
        pl->Preallocate();
        pSSEQ->Preallocate();
        pFIR->Preallocate();
        

        DBG("pre");
        pool->RegisterWork(0,pl);
        pool->RegisterWork(1,pSSEQ);
        pool->RegisterWork(2,pFIR);
        pool->RegisterWork(3,out);
        DBG("reg");
        pl->SetNextTrackCallback(NextTrackCallback,nullptr);
        DBG("regxx");
        pool->CreateThreads();
        DBG("player done");
        return pl;
    }
    void JoinPlayer()
    {
        pool->JoinThreads();
    }
    void SubmitForPlayback(Vrok::Resource *resource)
    {
        pl->SubmitForPlayback(resource);
    }
    void SubmitForPlaybackNow(Vrok::Resource *resource)
    {
        pl->SubmitForPlaybackNow(resource);
    }
    void PausePlayer()
    {
        pl->Pause();
    }
    void ResumePlayer()
    {
        pl->Resume();
    }
    void DeleteContext()
    {
        delete pl;
        delete out;
        delete pSSEQ;
        delete pool;
        delete pFIR;
    }
    void StopThreads()
    {
        pool->StopThreads();
    }
    float GetSSEQPreamp()
    {
        float val=0;
        Vrok::PropertyBase *p = Vrok::ComponentManager::GetSingleton()->GetProperty(
            pSSEQ,
            "preamp"
        );
        
        p->Get(&val);
        return val;
    }
    float GetSSEQBand(int band)
    {
        char bandname[32];
        snprintf(bandname,32,"band_%d",band);
        std::string sband=std::string(bandname);
        float val=0;
        DBG(bandname);
        Vrok::PropertyBase *p = Vrok::ComponentManager::GetSingleton()->GetProperty(
            pSSEQ,
            sband
        );
        p->Get(&val);
        return val;
    }
    void SetSSEQPreamp(float val)
    {
        
        Vrok::PropertyBase *p = Vrok::ComponentManager::GetSingleton()->GetProperty(
            pSSEQ,
            "preamp"
        );
        Vrok::ComponentManager::GetSingleton()->SetProperty(pSSEQ, p, &val);
        
    }
    void SetSSEQBand(int band, float val)
    {
        char bandname[32];
        snprintf(bandname,32,"band_%d",band);
        std::string sband=std::string(bandname);
        
        Vrok::PropertyBase *p = Vrok::ComponentManager::GetSingleton()->GetProperty(
            pSSEQ,
            sband
        );
        
        Vrok::ComponentManager::GetSingleton()->SetProperty(pSSEQ, p, &val);
        
        
        
    }
    
    
}



void NextTrackCallback(void *user) {

    if (hookObject) {
        JNIEnv *g_env;
        int getEnvStat = javaVM->GetEnv((void **)&g_env, JNI_VERSION_1_6);
        if (getEnvStat == JNI_EDETACHED) {
            if (javaVM->AttachCurrentThread(&g_env, NULL) != 0) {
                DBG("Failed to attach");
                return;
            }
        } else if (getEnvStat == JNI_OK) {
            //
        } else if (getEnvStat == JNI_EVERSION) {
            DBG("GetEnv: version not supported");
        }
        jstring str = (jstring) g_env->CallNonvirtualObjectMethod(hookObject, hookClass, hookMethod );
        //strcpy(url, g_env->GetStringUTFChars( str , NULL ) );
        DBG(g_env->GetStringUTFChars( str , NULL ));
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
    DBG("context");
    CreateContext();
    DBG("context done");
    return JNI_VERSION_1_6;
}
JNIEXPORT void JNICALL
JNI_OnUnLoad(JavaVM *vm, void *reserved) 
{ 
    
    DeleteContext();
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_hookCallback( JNIEnv* env,
                                             jobject thiz)
{
    DBG("hook called");
   
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
    DBG("player done");
	
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
    ResumePlayer();
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_pause( JNIEnv* env,
                                          jobject thiz
                                        )
{
    PausePlayer();
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

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_setComponent( JNIEnv* env,
                                                jobject thiz,
                                                jstring name)
{
    const char *szComp = env->GetStringUTFChars( name , NULL );
    SetCurrentComponent(szComp);
    
}


JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_setPropertyFloat( JNIEnv* env,
                                                jobject thiz,
                                                jstring name,
                                                float val
                                         )
{
    const char *szProp = env->GetStringUTFChars( name , NULL );
    SetPropertyFloat(szProp, val);
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
    return GetSSEQPreamp();
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_setEqualizerPreamp( JNIEnv* env,
                                                       jobject thiz,
                                                       float value
                                                     )
{
    SetSSEQPreamp(value);
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_setEqualizerBand( JNIEnv* env,
                                                     jobject thiz,
                                                     int band, 
                                                     float value
                                                   )
{
    SetSSEQBand(band, value);
}

JNIEXPORT float JNICALL
Java_com_mx_vrok_VrokService_getEqualizerBand( JNIEnv* env,
                                                     jobject thiz,
                                                     int band
                                                   )
{
    return GetSSEQBand(band);
}

JNIEXPORT void JNICALL
Java_com_mx_vrok_VrokService_shutdown( JNIEnv* env,
                                                     jobject thiz
                                                   )
{
    StopThreads();
    exit(0);
}

}


