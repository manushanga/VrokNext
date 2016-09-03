
import com.mx.vrok.VrokServices;

class test {

    static {
        System.loadLibrary("vrok_jni");
    }

    public static void main(String[] args) {
        TestEventCallback tc = new TestEventCallback();
        VrokServices services = new VrokServices(tc);

        services.open("/media/madura/Data1/Songs/Video/Hiru Unplugged EP 29 Karunarathna Divulgane _ 2016-07-15.mp4");
        services.setSamplerate(22000);
        services.createThreads();
        services.joinThreads();
    }
} 
