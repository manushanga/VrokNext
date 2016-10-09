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
JavaVM* g_VM=nullptr;

static Vrok::Player *pl=nullptr;
static Vrok::DriverJBufferOut *out=nullptr;
static Vrok::DriverAlsa *outAlsa=nullptr;
static Vrok::Resampler *pResampler=nullptr;
static Vrok::EffectSSEQ *pSSEQ=nullptr;
static Vrok::EffectFIR *pFIR=nullptr;
static Vrok::Component *current=nullptr;
static Vrok::ThreadPool *pool=nullptr;

static jmethodID onBuffer;
static jmethodID onBufferConfigChange;
static jobject objEventCallback;

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
    CBufferEvents() :
        m_samplerate(0),
        m_frames(0),
        m_channels(0)
    {}
    void OnBuffer(double* buffer)
    {
        DBG(5,"onBuffer");
        assert(m_frames != 0 && m_channels != 0);
        JNIInvoker invoker;

        jdoubleArray result;
        result = invoker.getEnv()->NewDoubleArray(m_frames * m_channels);

        invoker.getEnv()->SetDoubleArrayRegion(result, 0, m_frames * m_channels, buffer);
        invoker.getEnv()->CallVoidMethod(objEventCallback, onBuffer, result);
    }
    void OnBufferConfigChange(int frames, int samplerate, int channels)
    {
        DBG(5,"onBufferConfigChange");
        JNIInvoker invoker;
        DBG(9, invoker.getEnv());
        DBG(9, frames  << " "<< samplerate <<" "<< channels);
        m_samplerate = samplerate;
        m_frames = frames;
        m_channels = channels;
        invoker.getEnv()->CallVoidMethod(objEventCallback, onBufferConfigChange, frames, samplerate, channels);
    }
private:
    int m_samplerate;
    int m_channels;
    int m_frames;
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

void CreateContext()
{
    Vrok::Notify::GetInstance()->SetNotifier(new CNotifier());

    DBG(0,"Creating Context");
    pl = new Vrok::Player;
    outAlsa = new Vrok::DriverAlsa;
    out = new Vrok::DriverJBufferOut;
    pResampler = new Vrok::Resampler;
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

    pl->RegisterSink(pResampler);

    pResampler->RegisterSource(pl);

    pResampler->RegisterSink(outAlsa);
    pResampler->RegisterSink(out);

    outAlsa->RegisterSource(pResampler);
    out->RegisterSource(pResampler);



    DBG(0,"Reg");

    out->Preallocate();
    pl->Preallocate();
    outAlsa->Preallocate();
    pResampler->Preallocate();
    pSSEQ->Preallocate();
    pFIR->Preallocate();


    DBG(0,"pre");
    pool->RegisterWork(0,pl);
    pool->RegisterWork(1,pResampler);
    pool->RegisterWork(2,out);
    pool->RegisterWork(3,outAlsa);
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
void PlaySingleThread(Vrok::Resource *resource, bool withOutputPlayback)
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
    pResampler->BufferConfigChange(&bc);

    if (withOutputPlayback)
    {
        outAlsa->BufferConfigChange(&bc);
    }

    while (run)
    {
        run = dec->DecoderRun(b, &bc);
        barray[0] = b;

        pResampler->EffectRun(bout, barray, 1);

        if (bcOld1 != *bout->GetBufferConfig())
        {
            out->BufferConfigChange(bout->GetBufferConfig());
            out->SetOldBufferConfig(*bout->GetBufferConfig());
        }
        bcOld1 = *bout->GetBufferConfig();

        out->DriverRun(bout);


        if (withOutputPlayback)
        {
            if (bcOld2 != *bout->GetBufferConfig())
            {
                outAlsa->BufferConfigChange(bout->GetBufferConfig());
                outAlsa->SetOldBufferConfig(*bout->GetBufferConfig());
            }
            bcOld2 = *bout->GetBufferConfig();

            outAlsa->DriverRun(bout);
        }

    }
    delete b;
    delete bout;
    delete[] barray;
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
    delete outAlsa;
    delete pSSEQ;
    delete pResampler;
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
(JNIEnv *env, jobject th, jstring url, bool withOutputPlayback)
{
    const char *zPath = env->GetStringUTFChars( url , NULL );
    DBG(0, zPath);

    PlaySingleThread(CreateResource(zPath), withOutputPlayback);

    env->ReleaseStringUTFChars(url, zPath);

}

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_setupCallbacks
(JNIEnv *env, jobject th, jobject callback)
{
    DBG(0,"setting up");
    objEventCallback = env->NewGlobalRef(callback);
    jclass cls = (jclass) env->NewGlobalRef( (jobject) env->GetObjectClass(objEventCallback) );// env->GetObjectClass(callback);
    onBuffer = env->GetMethodID(cls, "onBuffer", "([D)V");
    onBufferConfigChange = env->GetMethodID(cls, "onBufferConfigChange", "(III)V");

    DBG(0, "onBuffer "<< (void*) onBuffer);
    DBG(0, "onBufferConfigChange "<< (void*) onBufferConfigChange);

    dynamic_cast<Vrok::DriverJBufferOut*>(out)->SetEvents(new CBufferEvents);

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
(JNIEnv *, jobject, int samplerate)
{
    Vrok::PropertyBase *p = Vrok::ComponentManager::GetSingleton()->GetProperty(
                                pResampler,
                                "output_samplerate"
                            );
    Vrok::ComponentManager::GetSingleton()->SetProperty(pResampler, p, std::to_string(samplerate));
    //out->SetOutputSamplerate(samplerate);
}
