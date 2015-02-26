package com.mingspy.mseg;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.List;

public abstract class MsegJNI {
	static {
		
		System.out
				.println("-------------loading libmsegjni.so-----------------");
		boolean loaded = false;
		System.out.println("try load libmsegjni from jar");
		try {
			InputStream in = MsegJNI.class.getClass()
					.getResource("/libmsegjni.so").openStream();
			File dll = File.createTempFile("libmsegjni", ".so");
			FileOutputStream out = new FileOutputStream(dll);

			int i;
			byte[] buf = new byte[1024];
			while ((i = in.read(buf)) != -1) {
				out.write(buf, 0, i);
			}

			in.close();
			out.close();
			dll.deleteOnExit();

			System.load(dll.toString());//
			loaded = true;
			System.out.println("load libmsegjni from jar succeed!");
		} catch (Exception e) {
			loaded = false;
			System.err.println("load libmsegjni from jar error!");
		}
		if (!loaded) {
			System.out.println(System.getProperty("java.library.path"));
			System.out.println("try load libmsegjni from java.library.path");
			System.loadLibrary("msegjni");
		}

		System.out
				.println("-------------loaded libmsegjni.so-----------------");
	}

	public static native List<Token> ConfigInit(String path);

	public static native List<Token> ConfigSet(String key, String val);

	public static native List<Token> MsegInit();

	/**
	 * Forward max match split.
	 * 
	 * @param str
	 * @return
	 */
	public static native List<Token> ForwardSplit(String str);

	/**
	 * ForWard full match split.
	 * 
	 * @param str
	 * @return
	 */
	public static native List<Token> FullSplit(String str);

	/**
	 * Using one gram split.
	 * 
	 * @param str
	 * @return
	 */
	public static native List<Token> SmartSplit(String str);

	public static native List<Token> BackwardSplit(String str);

	public static native List<Token> Tagging(String str);
}
