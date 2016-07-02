#include "jni_exposer.h"

#include <string>
#include <mutex>
#include <iostream>

#include "ffmpeg.h"
#include "alsa.h"
#include "player.h"
#include "jbufferout.h"
#include "preamp.h"
#include "fir.h"
#include "eq.h"
#include "threadpool.h"
#include "componentmanager.h"

JavaVM* g_VM=nullptr;

static Vrok::Player *pl=nullptr;
static Vrok::Driver *out=nullptr;
static Vrok::EffectSSEQ *pSSEQ=nullptr;
static Vrok::EffectFIR *pFIR=nullptr;
static Vrok::Component *current=nullptr;
static Vrok::ThreadPool *pool=nullptr;

class CNotifier : public Vrok::Notify::Notifier
{
public:
    void OnError(int level, std::string msg)
    {
        _guard.lock();
        std::cout<< "ERR: "<< msg << std::endl;
        _guard.unlock();
    }
    void OnWarning(int level, std::string msg)
    {
        _guard.lock();
        std::cout<< "WRN: "<< msg << std::endl;
        _guard.unlock();
    }
    void OnDebug(int level, std::string msg)
    {
        _guard.lock();
        std::cout<< "DBG: "<< msg << std::endl;
        _guard.unlock();
    }
    void OnInformation(std::string msg)
    {
        _guard.lock();
        std::cout<< "INF: "<< msg << std::endl;
        _guard.unlock();
    }
private:
    std::mutex _guard;
};

void CreateContext()
{
    Vrok::Notify::GetInstance()->SetNotifier(new CNotifier());

    DBG(0,"Creating Context");
    pl = new Vrok::Player;
    out = new Vrok::DriverAlsa;
    pSSEQ = new Vrok::EffectSSEQ;
    pFIR = new Vrok::EffectFIR;
    pool = new Vrok::ThreadPool(4);

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

    Vrok::ComponentManager::GetSingleton()->SetProperty(current, p, std::to_string(val));

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

void CreateThreads()
{
    DBG(0,"player");
    pFIR->RegisterSource(pl);
    pFIR->RegisterSink(pSSEQ);

    pSSEQ->RegisterSource(pFIR);
    pSSEQ->RegisterSink(out);

    pl->RegisterSink(pFIR);

    out->RegisterSource(pSSEQ);

    DBG(0,"Reg");

    out->Preallocate();
    pl->Preallocate();
    pSSEQ->Preallocate();
    pFIR->Preallocate();


    DBG(0,"pre");
    pool->RegisterWork(0,pl);
    pool->RegisterWork(1,pSSEQ);
    pool->RegisterWork(2,pFIR);
    pool->RegisterWork(3,out);
    DBG(0,"reg");
    //setEvents
    DBG(0,"regxx");
    pool->CreateThreads();
    DBG(0,"player done");
}

void JoinThreads()
{
    pool->JoinThreads();
}

void SubmitForPlayback(Vrok::Resource *resource)
{
    Vrok::Decoder* dec = new Vrok::DecoderFFMPEG();
    dec->Open(resource);
    pl->SubmitForPlayback(dec);
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
    DBG(0,bandname);
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
    Vrok::ComponentManager::GetSingleton()->SetProperty(pSSEQ, p, std::to_string(val));

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

    Vrok::ComponentManager::GetSingleton()->SetProperty(pSSEQ, p, std::to_string(val));



}


// --- JNI functions ---

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_open
(JNIEnv *env, jobject th, jstring url)
{
    const char *zPath = env->GetStringUTFChars( url , NULL );
    DBG(0, zPath);
    SubmitForPlayback(CreateResource(zPath));
    env->ReleaseStringUTFChars(url, zPath);
}

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_close
(JNIEnv *env, jobject th)
{

}

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_createThreads
  (JNIEnv *, jobject)
{
    CreateThreads();
}
JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_joinThreads
  (JNIEnv *, jobject)
{
    JoinThreads();
}
