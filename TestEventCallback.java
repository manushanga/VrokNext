
import com.mx.vrok.EventCallback;

public class TestEventCallback implements EventCallback
{
    public void onBufferConfigChange(int frames, int sampleRate, int channels)
    {
        System.out.println("bc");
    }
    
    public void onBuffer(double[] pcm)
    {
        for (int i=0;i<pcm.length;i++)
            System.out.println(pcm[i]);

    }
}
