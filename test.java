
import com.mx.vrok.VrokServices;

class test {

    static {
        System.loadLibrary("vrok_jni");
    }

    public static void main(String[] args) {
        TestEventCallback tc = new TestEventCallback();
        VrokServices services = new VrokServices(0,tc);
        
        services.setSamplerate(0,48000);
        services.openSingleThread("/home/madura/wrk/threadpool/d.mp3", 0, true);
        //services.openSingleThread("/home/madura/aaa/Alesis-Fusion-Clean-Guitar-C3.wav",true);
        //services.createThreads();
        //services.joinThreads();
    }
} 
