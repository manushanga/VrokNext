
import com.mx.vrok.EventCallback;

public class TestEventCallback implements EventCallback
{
    public void onBufferConfigChange(int frames, int sampleRate, int channels)
    {
        System.out.println("bc");
    }
    
    public void onBuffer(double[] pcm)
    {
        System.out.println("pcm");
    }
}
