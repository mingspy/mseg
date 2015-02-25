import java.io.*;
import com.mingspy.mseg.*;
import java.util.List;

public class Test {  
    public static void main(String[] args) throws InterruptedException {  
        System.err.println("-------------configing-----------------");
        MsegJNI.ConfigSet("CORE_DICT_PATH","/data0/home/xiulei/workspace/mseg/test/core.dic");
        System.err.println("-------------initing mseg-----------------");
        MsegJNI.MsegInit();
        System.err.println("-------------call seg-----------------");
        List<Token> ls = MsegJNI.ForwardSplit("他说的确实在理123 abc.com");
        for (Token t : ls){
            System.out.println(t);
        }

    }
} 
