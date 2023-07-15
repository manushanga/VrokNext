#include <jni.h>

#include "audiotrack.h"

#define AT_STREAM_MUSIC 3
#define AT_CHANNEL_CONFIGURATION_MONO 2
#define AT_CHANNEL_CONFIGURATION_STEREO 3
#define AT_ENCODING_PCM_8BIT 3
#define AT_ENCODING_PCM_16BIT 2
#define AT_MODE_STREAM 1

JavaVM *javaVM = NULL;

static jclass cAudioTrack = NULL;

static jmethodID mAudioTrack;
static jmethodID mGetMinBufferSize;
static jmethodID mPlay;
static jmethodID mPause;
static jmethodID mStop;
static jmethodID mRelease;
static jmethodID mWrite;
static jmethodID mFlush;
static jmethodID mSetStereoVolume;
static jarray jbuffer;
static jobject jtrack;
static JNIEnv *jenv;

static JNIEnv *GetEnv() {
    JNIEnv *jenv = NULL;
    int err = javaVM->GetEnv((void **)&jenv, JNI_VERSION_1_6);
    if (err == JNI_EDETACHED) {
        int err = javaVM->AttachCurrentThread(&jenv, NULL);
        if (err != 0) {
            DBG("Attach failed");
        }
    } else if (err != JNI_OK) {
        DBG("Can't get JNI env");
    }
    return jenv;
}

void vrok::DriverAudioTrack::initAudioTrack(BufferConfig *config) {
    jenv = GetEnv();

    if (!cAudioTrack) {
        /* Cache AudioTrack class and it's method id's
         * And do this only once!
         */

        cAudioTrack = jenv->FindClass("android/media/AudioTrack");
        if (!cAudioTrack) {
            DBG("can't get AudioTrack class");
            return;
        }

        cAudioTrack = (jclass)jenv->NewGlobalRef(cAudioTrack);

        mAudioTrack = jenv->GetMethodID(cAudioTrack, "<init>", "(IIIIII)V");
        mGetMinBufferSize = jenv->GetStaticMethodID(cAudioTrack, "getMinBufferSize", "(III)I");
        mPlay = jenv->GetMethodID(cAudioTrack, "play", "()V");
        mPause = jenv->GetMethodID(cAudioTrack, "pause", "()V");
        mStop = jenv->GetMethodID(cAudioTrack, "stop", "()V");
        mRelease = jenv->GetMethodID(cAudioTrack, "release", "()V");
        mWrite = jenv->GetMethodID(cAudioTrack, "write", "([BII)I");
        mFlush = jenv->GetMethodID(cAudioTrack, "flush", "()V");
        mSetStereoVolume = jenv->GetMethodID(cAudioTrack, "setStereoVolume", "(FF)I");
    }

    int channelConfig =
            (config->channels == 1) ? AT_CHANNEL_CONFIGURATION_MONO : AT_CHANNEL_CONFIGURATION_STEREO;
    int bufferSize = config->frames * sizeof(short) * config->channels;

    int bufferSizeInBytes = jenv->CallStaticIntMethod(cAudioTrack, mGetMinBufferSize, config->samplerate,
                                                      channelConfig, AT_ENCODING_PCM_16BIT);

    //     if (bufferSizeInBytes > 2*bufferSize) {
    //             DBG("Buffer too small");
    //             exit(0);
    //     }

    jtrack = jenv->NewObject(cAudioTrack, mAudioTrack, AT_STREAM_MUSIC, config->samplerate, channelConfig,
                             AT_ENCODING_PCM_16BIT, 4 * bufferSizeInBytes, AT_MODE_STREAM);
    jtrack = jenv->NewGlobalRef(jtrack);

    jenv->CallNonvirtualVoidMethod(jtrack, cAudioTrack, mPlay);

    jbuffer = jenv->NewByteArray(bufferSize);

    jbuffer = (jarray)jenv->NewGlobalRef((jobject)jbuffer);

    _init = true;
    javaVM->DetachCurrentThread();
}
vrok::DriverAudioTrack::~DriverAudioTrack() { }
void vrok::DriverAudioTrack::finiAudioTrack(BufferConfig *config) {
    jenv = GetEnv();
    if (_init) {

        jenv->CallNonvirtualVoidMethod(jtrack, cAudioTrack, mPause);

        jenv->CallNonvirtualVoidMethod(jtrack, cAudioTrack, mFlush);
        jenv->CallNonvirtualVoidMethod(jtrack, cAudioTrack, mRelease);

        jenv->DeleteGlobalRef(jtrack);
        jenv->DeleteGlobalRef((jobject)jbuffer);
    }
    javaVM->DetachCurrentThread();
}
vrok::DriverAudioTrack::DriverAudioTrack() : _init(false) {

    /*ao_initialize();

    ao_sample_format sformat;jenv = GetEnv();
    initAudioTrack(GetBufferConfig());
atteempt to use stale local reference
    sformat.channels=GetBufferConfig()->channels;
    sformat.rate=GetBufferConfig()->samplerate;
    sformat.bits=16;
    sformat.byte_format=AO_FMT_NATIVE;
    sformat.matrix=0;

    _ao_device=ao_open_live(ao_default_driver_id(),&sformat,NULL);*/
}

void vrok::DriverAudioTrack::ThreadStart() {
    DBG("attaching");
    // javaVM->AttachCurrentThread(&jenv, NULL);
}

void vrok::DriverAudioTrack::ThreadEnd() {
    // javaVM->DetachCurrentThread();
    DBG("detaching");

    // javaVM->DetachCurrentThread();
}
bool vrok::DriverAudioTrack::BufferConfigChange(BufferConfig *config) {
    if (*GetBufferConfig() != *config) {
        finiAudioTrack(config);
        initAudioTrack(config);
        DBG("o " << config->channels);

        DBG("o " << config->samplerate);
    }
    return true;
}
bool vrok::DriverAudioTrack::DriverRun(Buffer *buffer) {
    int samples = GetBufferConfig()->channels * GetBufferConfig()->frames;

    jenv = GetEnv();

    short *pBuffer = (short *)jenv->GetPrimitiveArrayCritical(jbuffer, NULL);
    if (pBuffer) {
        for (int i = 0; i < samples; i++) {
            pBuffer[i] = (short)(buffer->GetData()[i] * 32765.0f);
        }
        jenv->ReleasePrimitiveArrayCritical(jbuffer, pBuffer, 0);
        jenv->CallNonvirtualIntMethod(jtrack, cAudioTrack, mWrite, jbuffer, 0, samples * sizeof(short));
    } else {
        DBG("Can't get buffer pointer");
    }

    javaVM->DetachCurrentThread();
    return true;
}
