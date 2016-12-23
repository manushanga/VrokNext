package com.mx.vrok;

import com.mx.vrok.EventCallback;
import java.lang.String;

public class VrokServices
{
    public native void createThreads();
    public native void joinThreads();
    public native void open(String url);
    public native void openSingleThread(String url, int thread, boolean withOutputPlayback);
    public native void close();
    public native void initThread(int thread);
    public native void setupCallbacks(int thread, EventCallback callback);
    public native void setSamplerate(int thread, int samplerate);
    
    public VrokServices(int thread, EventCallback callback)
    {
        m_callback = callback;
        initThread(thread);
        setupCallbacks(thread, callback);
    }
    private void onBufferConfigChange(int frames, int samplerate, int channels)
    {
        m_callback.onBufferConfigChange(frames, samplerate, channels);
    }
    private void onBuffer(double[] pcm)
    {
        m_callback.onBuffer(pcm);
    }
    
    private EventCallback m_callback;
}
