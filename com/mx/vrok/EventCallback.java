package com.mx.vrok;

public interface EventCallback
{
    public void onBufferConfigChange(int frames, int sampleRate, int channels);
    
    public void onBuffer(double[] pcm);
}
