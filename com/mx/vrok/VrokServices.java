package com.mx.vrok;

import com.mx.vrok.EventCallback;
import java.lang.String;

public class VrokServices
{
    public native void createThreads();
    public native void joinThreads();
    public native void open(String url);
    public native void close();
    public native void setupCallbacks(EventCallback callback);
    public native void setSamplerate(int samplerate);
    
    public VrokServices(EventCallback callback)
    {
        m_callback = callback;
        setupCallbacks(callback);
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
