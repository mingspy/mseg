package com.mingspy;
import java.io.*;

import com.mingspy.mseg.*;

import java.util.List;

public class Test {  
    public static void main(String[] args) throws InterruptedException, FileNotFoundException {  
        System.err.println("-------------configing-----------------");
        MsegJNI.setDictRoot("/data0/home/xiulei/workspace/mseg/test/");
        System.err.println("-------------initing mseg-----------------");
        MsegJNI.init();
        System.err.println("-------------call seg-----------------");
        List<Token> ls = MsegJNI.ForwardSplit("他说的确实在理123 abc.com");
        for (Token t : ls){
            System.out.println(t);
        }

    }
} 
