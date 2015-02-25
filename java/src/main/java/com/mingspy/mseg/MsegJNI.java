package com.mingspy.mseg;

import java.util.List;

public abstract class MsegJNI
{
    static
    {
        System.out.println(System.getProperty("java.library.path"));
        System.out.println("-------------loading libmsegjni.so-----------------");
        System.loadLibrary("msegjni");
        System.out.println("-------------loaded libmsegjni.so-----------------");
    }

    public static native List<Token> ConfigInit(String path);
    public static native List<Token> ConfigSet(String key,String val);
    public static native List<Token> MsegInit();

    /**
     * Forward max match split.
     * @param str
     * @return
     */
    public static native List<Token> ForwardSplit(String str);


    /**
     * ForWard full match split.
     * @param str
     * @return
     */
    public static native List<Token> FullSplit(String str);

    /**
     * Using one gram split.
     * @param str
     * @return
     */
    public static native List<Token> SmartSplit(String str);

    public static native List<Token> BackwardSplit(String str);

    public static native List<Token> Tagging(String str);
}
