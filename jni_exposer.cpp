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
#include "resampler.h"
#include "threadpool.h"
#include "componentmanager.h"

#include "buffer.h"
#include "bufferconfig.h"

#define MAX_THREADS 32

JavaVM* g_VM=nullptr;

static Vrok::Player *pPlayer[MAX_THREADS] = {nullptr};
static Vrok::DriverJBufferOut *pJavaOut[MAX_THREADS] = {nullptr};
static Vrok::DriverAlsa *pOutAlsa = nullptr;
static Vrok::Resampler *pResampler[MAX_THREADS] = {nullptr};
static Vrok::EffectSSEQ *pSSEQ=nullptr;
static Vrok::EffectFIR *pFIR=nullptr;
static Vrok::Component *current=nullptr;
static Vrok::ThreadPool *pool=nullptr;

static jmethodID onBuffer;
static jmethodID onBufferConfigChange;
static jobject objEventCallback[MAX_THREADS];

static bool g_withOutputPlayback = false;

class JNIInvoker
{
public:
    JNIInvoker()
    {
        int getEnvStat = g_VM->GetEnv((void **)&m_env, JNI_VERSION_1_6);
        if (getEnvStat == JNI_EDETACHED)
        {
            if (g_VM->AttachCurrentThread((void **) &m_env, NULL) != 0)
            {
                std::cerr << "Failed to attach" << std::endl;
            }
        }
        else if (getEnvStat == JNI_OK)
        {
        }
        else if (getEnvStat == JNI_EVERSION)
        {
            std::cerr << "GetEnv: version not supported" << std::endl;
        }
    }
    JNIEnv* getEnv()
    {
        return m_env;
    }
    ~JNIInvoker()
    {
        g_VM->DetachCurrentThread();
    }
private:
    JNIEnv *m_env;
};
class CBufferEvents : public Vrok::DriverJBufferOut::Events
{
public:
    CBufferEvents(int thread) :
        m_samplerate(0),
        m_frames(0),
        m_channels(0),
        m_thread(thread)
    {}
    void OnBuffer(double* buffer)
    {
        DBG(5,"onBuffer tid:" <<m_thread);
        assert(m_frames != 0 && m_channels != 0);
        JNIInvoker invoker;

        jdoubleArray result;
        result = invoker.getEnv()->NewDoubleArray(m_frames * m_channels);

        invoker.getEnv()->SetDoubleArrayRegion(result, 0, m_frames * m_channels, buffer);
        invoker.getEnv()->CallVoidMethod(objEventCallback[m_thread], onBuffer, result);
    }
    void OnBufferConfigChange(int frames, int samplerate, int channels)
    {
        DBG(5,"onBufferConfigChange tid:" <<m_thread);
        JNIInvoker invoker;
        DBG(9, invoker.getEnv());
        DBG(9, frames  << " "<< samplerate <<" "<< channels);
        m_samplerate = samplerate;
        m_frames = frames;
        m_channels = channels;
        invoker.getEnv()->CallVoidMethod(objEventCallback[m_thread], onBufferConfigChange, frames, samplerate, channels);
    }
private:
    int m_samplerate;
    int m_channels;
    int m_frames;
    int m_thread;
};
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

void CreateThreadContext(int thread)
{
    pPlayer[thread] = new Vrok::Player;
    pJavaOut[thread] = new Vrok::DriverJBufferOut;
    pResampler[thread] = new Vrok::Resampler;
}
void CreateContext()
{
    Vrok::Notify::GetInstance()->SetNotifier(new CNotifier());

    DBG(0,"Creating Context");
    pPlayer[0] = new Vrok::Player;
    pOutAlsa = new Vrok::DriverAlsa;
    pJavaOut[0] = new Vrok::DriverJBufferOut;
    pResampler[0] = new Vrok::Resampler;
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
    //pFIR->RegisterSource(pl);
    //pl->RegisterSink(pFIR);

    //pFIR->RegisterSink(pResampler);
    //pResampler->RegisterSource(pFIR);

    //pSSEQ->RegisterSource(pFIR);
    //pFIR->RegisterSink(pSSEQ);

    //pSSEQ->RegisterSink(out);
    //out->RegisterSource(pSSEQ);

    //outAlsa->RegisterSource(pResampler);
    //pResampler->RegisterSink(outAlsa);

    pPlayer[0]->RegisterSink(pResampler[0]);

    pResampler[0]->RegisterSource(pPlayer[0]);

    pResampler[0]->RegisterSink(pOutAlsa);
    pResampler[0]->RegisterSink(pJavaOut[0]);

    pOutAlsa->RegisterSource(pResampler[0]);
    pJavaOut[0]->RegisterSource(pResampler[0]);



    DBG(0,"Reg");

    pJavaOut[0]->Preallocate();
    pPlayer[0]->Preallocate();
    pOutAlsa->Preallocate();
    pResampler[0]->Preallocate();
    pSSEQ->Preallocate();
    pFIR->Preallocate();


    DBG(0,"pre");
    pool->RegisterWork(0,pPlayer[0]);
    pool->RegisterWork(1,pResampler[0]);
    pool->RegisterWork(2,pJavaOut[0]);
    pool->RegisterWork(3,pOutAlsa);
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
    pPlayer[0]->SubmitForPlayback(dec);
}
void PlaySingleThread(Vrok::Resource *resource, int thread, bool withOutputPlayback)
{
    DBG(0, "single thread mode");
    Vrok::Decoder* dec = new Vrok::DecoderFFMPEG();
    dec->Open(resource);
    BufferConfig bc,bcOld1(0,0,0),bcOld2(0,0,0);
    dec->GetBufferConfig(&bc);
    Buffer* b = new Buffer(bc,0);
    Buffer* bout = new Buffer(bc,0);

    Buffer** barray = new Buffer*[1];
    bool run = dec->DecoderRun(b, &bc);
    pResampler[thread]->BufferConfigChange(&bc);

    if (withOutputPlayback && thread == 0)
    {
        pOutAlsa->BufferConfigChange(&bc);
    }

    while (run)
    {
        run = dec->DecoderRun(b, &bc);
        barray[0] = b;

        pResampler[thread]->EffectRun(bout, barray, 1);

        if (bcOld1 != *bout->GetBufferConfig())
        {
            pJavaOut[thread]->BufferConfigChange(bout->GetBufferConfig());
            pJavaOut[thread]->SetOldBufferConfig(*bout->GetBufferConfig());
        }
        bcOld1 = *bout->GetBufferConfig();

        pJavaOut[thread]->DriverRun(bout);


        if (withOutputPlayback && thread == 0)
        {
            if (bcOld2 != *bout->GetBufferConfig())
            {
                pOutAlsa->BufferConfigChange(bout->GetBufferConfig());
                pOutAlsa->SetOldBufferConfig(*bout->GetBufferConfig());
            }
            bcOld2 = *bout->GetBufferConfig();

            pOutAlsa->DriverRun(bout);
        }

    }
    delete b;
    delete bout;
    delete[] barray;
    delete dec;
}
void PausePlayer()
{
    pPlayer[0]->Pause();
}
void ResumePlayer()
{
    pPlayer[0]->Resume();
}
void DeleteContext()
{
    delete pPlayer[0];
    delete pJavaOut[0];
    delete pOutAlsa;
    delete pSSEQ;
    delete pResampler[0];
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

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_openSingleThread
(JNIEnv *env, jobject th, jstring url, int thread, bool withOutputPlayback)
{
    const char *zPath = env->GetStringUTFChars( url , NULL );
    DBG(0, zPath);

    PlaySingleThread(CreateResource(zPath), thread, withOutputPlayback);

    env->ReleaseStringUTFChars(url, zPath);

}

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_initThread
(JNIEnv *env, jobject th, int thread )
{
    CreateThreadContext(thread);
}

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_setupCallbacks
(JNIEnv *env, jobject th, int thread, jobject callback)
{
    DBG(0,"setting up");
    objEventCallback[thread] = env->NewGlobalRef(callback);
    jclass cls = (jclass) env->NewGlobalRef( (jobject) env->GetObjectClass(objEventCallback[thread]) );// env->GetObjectClass(callback);
    onBuffer = env->GetMethodID(cls, "onBuffer", "([D)V");
    onBufferConfigChange = env->GetMethodID(cls, "onBufferConfigChange", "(III)V");

    DBG(0, "onBuffer "<< (void*) onBuffer);
    DBG(0, "onBufferConfigChange "<< (void*) onBufferConfigChange);

    dynamic_cast<Vrok::DriverJBufferOut*>(pJavaOut[thread])->SetEvents(new CBufferEvents(thread));

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

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_setSamplerate
(JNIEnv *, jobject, int thread, int samplerate)
{
    Vrok::PropertyBase *p = Vrok::ComponentManager::GetSingleton()->GetProperty(
                                pResampler[thread],
                                "output_samplerate"
                            );
    Vrok::ComponentManager::GetSingleton()->SetProperty(pResampler[thread], p, std::to_string(samplerate));
    //out->SetOutputSamplerate(samplerate);
}
