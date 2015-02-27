package com.mingspy.mseg;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.List;

public abstract class MsegJNI {
	public static final String KEY_DICT_ROOT="DICT_ROOT";
	public static final String KEY_CONF_PATH="CONF_PATH";
	public static final String KEY_CORE_DICT_NAME="CORE_DICT_NAME";
	public static final String KEY_INVS_DICT_NAME="INVS_DICT_NAME";
	public static final String KEY_USER_DICT_DIR="USER_DICT_DIR";
	public static final String KEY_IS_LOAD_INVS="IS_LOAD_INVS";
	
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

	private static native List<Token> ConfigInit(String path);

	private static native List<Token> ConfigSet(String key, String val);

	private static native List<Token> MsegInit();

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
	
	private static boolean is_inited = false;

	public static synchronized boolean init() {
		if(!is_inited){
			MsegInit();
			is_inited = true;
		}
		return is_inited;
	}
	
	public static synchronized boolean setConfigPath(String path) throws FileNotFoundException{
		if(!is_inited){
			File file =new File(path);      
			if  (!file.exists() || !file.isFile())      
			{       
			    throw new FileNotFoundException(path);
			}
			ConfigInit(path);
			return true;
		}
		return false;
	}
	
	public static synchronized boolean setProperties(String key,String val){
		if(!is_inited){
			ConfigSet(key,val);
			return true;
		}
		return false;
	}
	
	public static boolean isLoadInverseDict(boolean load){
		if(load){
			return setProperties(KEY_IS_LOAD_INVS,"true");
		}else{
			return setProperties(KEY_IS_LOAD_INVS,"false");
		}
	}
	
	public static boolean setDictRoot(String path) throws FileNotFoundException{
		File file =new File(path);      
		if  (!file.exists() || !file.isDirectory())      
		{       
		    throw new FileNotFoundException(path);
		}
		return setProperties(KEY_DICT_ROOT,path);
	}
	
	public static boolean setUserDictDir(String dir){
		return setProperties(KEY_USER_DICT_DIR,dir);
	}
	
	public static boolean setCoreDictName(String name){
		return setProperties(KEY_CORE_DICT_NAME,name);
	}
	
	public static boolean setInverseDictName(String name){
		return setProperties(KEY_INVS_DICT_NAME,name);
	}
}
