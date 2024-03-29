#include "jni_exposer.h"

#include <iostream>
#include <mutex>
#include <string>

#include "alsa.h"
#include "componentmanager.h"
#include "eq.h"
#include "ffmpeg.h"
#include "fir.h"
#include "jbufferout.h"
#include "notifier_impl.h"
#include "player.h"
#include "preamp.h"
#include "resampler.h"
#include "threadpool.h"

JavaVM *g_VM = nullptr;

static vrok::Player *pl = nullptr;
static vrok::DriverJBufferOut *out = nullptr;
static vrok::DriverAlsa *outAlsa = nullptr;
static vrok::Resampler *pResampler = nullptr;
static vrok::EffectSSEQ *pSSEQ = nullptr;
static vrok::EffectFIR *pFIR = nullptr;
static vrok::Component *current = nullptr;
static vrok::ThreadPool *pool = nullptr;

static jmethodID onBuffer;
static jmethodID onBufferConfigChange;
static jobject objEventCallback;

class JNIInvoker {
public:
    JNIInvoker() {
        int getEnvStat = g_VM->GetEnv((void **)&m_env, JNI_VERSION_1_6);
        if (getEnvStat == JNI_EDETACHED) {
            if (g_VM->AttachCurrentThread((void **)&m_env, NULL) != 0) {
                std::cerr << "Failed to attach" << std::endl;
            }
        } else if (getEnvStat == JNI_OK) {
        } else if (getEnvStat == JNI_EVERSION) {
            std::cerr << "GetEnv: version not supported" << std::endl;
        }
    }
    JNIEnv *getEnv() { return m_env; }
    ~JNIInvoker() { g_VM->DetachCurrentThread(); }

private:
    JNIEnv *m_env;
};
class CBufferEvents : public vrok::DriverJBufferOut::Events {
public:
    CBufferEvents() : m_samplerate(0), m_frames(0), m_channels(0) { }
    void OnBuffer(double *buffer) {
        DBG(0, "onBuffer");
        assert(m_frames != 0 && m_channels != 0);
        JNIInvoker invoker;

        jdoubleArray result;
        result = invoker.getEnv()->NewDoubleArray(m_frames * m_channels);

        invoker.getEnv()->SetDoubleArrayRegion(result, 0, m_frames * m_channels, buffer);
        invoker.getEnv()->CallVoidMethod(objEventCallback, onBuffer, result);
    }
    void OnBufferConfigChange(int frames, int samplerate, int channels) {
        DBG(0, "onBufferConfigChange");
        JNIInvoker invoker;
        DBG(0, invoker.getEnv());
        DBG(0, frames << " " << samplerate << " " << channels);
        m_samplerate = samplerate;
        m_frames = frames;
        m_channels = channels;
        invoker.getEnv()->CallVoidMethod(objEventCallback, onBufferConfigChange, frames, samplerate,
                                         channels);
    }

private:
    int m_samplerate;
    int m_channels;
    int m_frames;
};

void CreateContext() {
    vrok::Notify::GetInstance()->SetNotifier(new CNotifier());

    DBG(0, "Creating Context");
    pl = new vrok::Player;
    outAlsa = new vrok::DriverAlsa;
    out = new vrok::DriverJBufferOut;
    pResampler = new vrok::Resampler;
    pSSEQ = new vrok::EffectSSEQ;
    pFIR = new vrok::EffectFIR;
    pool = new vrok::ThreadPool(4);
}

void SetCurrentComponent(const char *name) {
    current = vrok::ComponentManager::GetSingleton()->GetComponent(std::string(name));
}

void SetPropertyFloat(const char *name, float val) {
    vrok::PropertyBase *p = vrok::ComponentManager::GetSingleton()->GetProperty(current, std::string(name));

    vrok::ComponentManager::GetSingleton()->SetProperty(current, p, std::to_string(val));
}

vrok::Resource *CreateResource(const char *filename) {
    vrok::Resource *res = new vrok::Resource();
    res->_filename = std::string(filename);
    return res;
}
void DestroyResource(vrok::Resource *resource) {
    delete resource;
}
void RegisterSource(BufferGraph::Point *parent, BufferGraph::Point *source) {
    parent->RegisterSource(source);
}
void RegisterSink(BufferGraph::Point *parent, BufferGraph::Point *sink) {
    parent->RegisterSource(sink);
}

void CreateThreads() {
    DBG(0, "player");
    // pFIR->RegisterSource(pl);
    // pl->RegisterSink(pFIR);

    // pFIR->RegisterSink(pResampler);
    // pResampler->RegisterSource(pFIR);

    // pSSEQ->RegisterSource(pFIR);
    // pFIR->RegisterSink(pSSEQ);

    // pSSEQ->RegisterSink(out);
    // out->RegisterSource(pSSEQ);

    // outAlsa->RegisterSource(pResampler);
    // pResampler->RegisterSink(outAlsa);

    pl->RegisterSink(pResampler);

    pResampler->RegisterSource(pl);

    pResampler->RegisterSink(outAlsa);
    pResampler->RegisterSink(out);

    outAlsa->RegisterSource(pResampler);
    out->RegisterSource(pResampler);

    DBG(0, "Reg");

    out->Preallocate();
    pl->Preallocate();
    outAlsa->Preallocate();
    pResampler->Preallocate();
    pSSEQ->Preallocate();
    pFIR->Preallocate();

    DBG(0, "pre");
    pool->RegisterWork(0, pl);
    pool->RegisterWork(1, pResampler);
    pool->RegisterWork(2, out);
    pool->RegisterWork(3, outAlsa);
    DBG(0, "reg");
    // setEvents
    DBG(0, "regxx");
    pool->CreateThreads();
    DBG(0, "player done");
}

void JoinThreads() {
    pool->JoinThreads();
}

void SubmitForPlayback(vrok::Resource *resource) {
    vrok::Decoder *dec = new vrok::DecoderFFMPEG();
    dec->Open(resource);
    pl->SubmitForPlayback(dec);
}

void PausePlayer() {
    pl->Pause();
}
void ResumePlayer() {
    pl->Resume();
}
void DeleteContext() {
    delete pl;
    delete out;
    delete outAlsa;
    delete pSSEQ;
    delete pResampler;
    delete pool;
    delete pFIR;
}
void StopThreads() {
    pool->StopThreads();
}
float GetSSEQPreamp() {
    float val = 0;
    vrok::PropertyBase *p = vrok::ComponentManager::GetSingleton()->GetProperty(pSSEQ, "preamp");

    p->Get(&val);
    return val;
}
float GetSSEQBand(int band) {
    char bandname[32];
    snprintf(bandname, 32, "band_%d", band);
    std::string sband = std::string(bandname);
    float val = 0;
    DBG(0, bandname);
    vrok::PropertyBase *p = vrok::ComponentManager::GetSingleton()->GetProperty(pSSEQ, sband);
    p->Get(&val);
    return val;
}
void SetSSEQPreamp(float val) {

    vrok::PropertyBase *p = vrok::ComponentManager::GetSingleton()->GetProperty(pSSEQ, "preamp");
    vrok::ComponentManager::GetSingleton()->SetProperty(pSSEQ, p, std::to_string(val));
}
void SetSSEQBand(int band, float val) {
    char bandname[32];
    snprintf(bandname, 32, "band_%d", band);
    std::string sband = std::string(bandname);

    vrok::PropertyBase *p = vrok::ComponentManager::GetSingleton()->GetProperty(pSSEQ, sband);

    vrok::ComponentManager::GetSingleton()->SetProperty(pSSEQ, p, std::to_string(val));
}

// --- JNI functions ---

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_open(JNIEnv *env, jobject th, jstring url) {
    const char *zPath = env->GetStringUTFChars(url, NULL);
    DBG(0, zPath);
    SubmitForPlayback(CreateResource(zPath));
    env->ReleaseStringUTFChars(url, zPath);
}

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_setupCallbacks(JNIEnv *env, jobject th,
                                                                    jobject callback) {
    DBG(0, "setting up");
    objEventCallback = env->NewGlobalRef(callback);
    jclass cls = (jclass)env->NewGlobalRef(
            (jobject)env->GetObjectClass(objEventCallback)); // env->GetObjectClass(callback);
    onBuffer = env->GetMethodID(cls, "onBuffer", "([D)V");
    onBufferConfigChange = env->GetMethodID(cls, "onBufferConfigChange", "(III)V");

    DBG(0, "onBuffer " << (void *)onBuffer);
    DBG(0, "onBufferConfigChange " << (void *)onBufferConfigChange);

    dynamic_cast<Vrok::DriverJBufferOut *>(out)->SetEvents(new CBufferEvents);
}

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_close(JNIEnv *env, jobject th) { }

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_createThreads(JNIEnv *, jobject) {
    CreateThreads();
}
JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_joinThreads(JNIEnv *, jobject) {
    JoinThreads();
}

JNIEXPORT void JNICALL Java_com_mx_vrok_VrokServices_setSamplerate(JNIEnv *, jobject, int samplerate) {
    Vrok::PropertyBase *p =
            Vrok::ComponentManager::GetSingleton()->GetProperty(pResampler, "output_samplerate");
    Vrok::ComponentManager::GetSingleton()->SetProperty(pResampler, p, std::to_string(samplerate));
    // out->SetOutputSamplerate(samplerate);
}
