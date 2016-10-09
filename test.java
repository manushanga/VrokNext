
import com.mx.vrok.VrokServices;

class test {

    static {
        System.loadLibrary("vrok_jni");
    }

    public static void main(String[] args) {
        TestEventCallback tc = new TestEventCallback();
        VrokServices services = new VrokServices(tc);
        
        services.setSamplerate(48000);
        services.openSingleThread("/home/madura/wrk/threadpool/d.mp3", true);
        //services.openSingleThread("/home/madura/aaa/Alesis-Fusion-Clean-Guitar-C3.wav",true);
        //services.createThreads();
        //services.joinThreads();
    }
} 
