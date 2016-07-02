
import com.mx.vrok.VrokServices;

class test {

    static {
        System.loadLibrary("vrok_jni");
    }

    public static void main(String[] args) {
        TestEventCallback tc = new TestEventCallback();
        VrokServices services = new VrokServices(tc);

        services.open("http://104.238.193.114:7034");
        services.createThreads();
        services.joinThreads();
    }
} 
