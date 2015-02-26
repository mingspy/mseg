#include <iostream>
#include <string.h>
#include <jni.h>
#include "mseg.cpp"

using namespace std;

static jobject toJavaTokenList(JNIEnv * env, const char * utf8str, 
        const Token * result, int len, bool addNature = false)
{
    jclass list_cls = env->FindClass("Ljava/util/ArrayList;");//获得ArrayList类引用
    jmethodID list_costruct_id = env->GetMethodID(list_cls , "<init>","()V"); //获得得构造函数Id
    jmethodID list_add  = env->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
    jobject list_obj = env->NewObject(list_cls , list_costruct_id);
    jclass token_cls = env->FindClass("Lcom/mingspy/mseg/Token;");
    jmethodID token_costruct_id = env->GetMethodID(token_cls , "<init>", 
            "(IILjava/lang/String;Ljava/lang/String;)V");
    jfieldID token_nature_id = env->GetFieldID(token_cls,"nature","Ljava/lang/String;");
    int unicode_start = 0;
    int unicode_end = 0; 
    char word[256];
    for(int i = 0 ; i < len; i++) {
        int len = result[i].end - result[i].start;
		if(len > 255) len = 255;
        strncpy(word,utf8str+result[i].start, len);
        word[len] = 0;
        unicode_end = utf8_to_unicode_len(utf8str,result[i].start,result[i].end) + unicode_start;
        jobject t_obj = env->NewObject(token_cls , token_costruct_id , unicode_start,
                unicode_end,env->NewStringUTF(word),NULL);
        unicode_start = unicode_end;
        if(addNature) {
            const char * nature = mseg_get_pos(result[i].pos);
            env->SetObjectField(t_obj, token_nature_id, env->NewStringUTF(nature));
        }
        env->CallBooleanMethod(list_obj , list_add , t_obj);
    }
    return list_obj;
}

#define CALL_MSEG_METHOD(env,jstr,func) \
	Token result[MIN_TOKEN_BUFSIZE]; \
    const char * p = env->GetStringUTFChars(jstr, 0); \
    int len = func(p, result, MIN_TOKEN_BUFSIZE); \
    jobject ret = toJavaTokenList(env,p, result, len); \
    env->ReleaseStringUTFChars(jstr, p); \
    return ret; 
/*
 * Class:     com_mingspy_jseg_JSegJNI
 * Method:    MaxSplit
 * Signature: (Ljava/lang/String;)Ljava/util/List;
 */
jobject JNI_ForwardSplit
(JNIEnv * env, jobject obj, jstring jstr)
{
    /*
    cerr<<"cpp ->Forward split"<<endl;
	struct Token result[MIN_TOKEN_BUFSIZE];
    const char * p = env->GetStringUTFChars(jstr, 0);
    int len = mseg_forward_split(p, result, MIN_TOKEN_BUFSIZE);
    jobject ret = toJavaTokenList(env,p, result, len);
    env->ReleaseStringUTFChars(jstr, p);
    return ret;
    */
    CALL_MSEG_METHOD(env,jstr,mseg_forward_split);
}

/*
 * Class:     com_mingspy_jseg_JSegJNI
 * Method:    BackwardSplit
 * Signature: (Ljava/lang/String;)Ljava/util/List;
 */
jobject JNI_BackwardSplit
(JNIEnv * env, jobject obj, jstring jstr)
{
    /*
	Token result[MIN_TOKEN_BUFSIZE];
    const char * p = env->GetStringUTFChars(jstr, 0);
    int len = mseg_backward_split(p, result, MIN_TOKEN_BUFSIZE);
    jobject ret = toJavaTokenList(env,p, result, len);
    env->ReleaseStringUTFChars(jstr, p);
    return ret;
    */
    CALL_MSEG_METHOD(env,jstr,mseg_backward_split);
}

/*
 * Class:     com_mingspy_jseg_JSegJNI
 * Method:    FullSplit
 * Signature: (Ljava/lang/String;)Ljava/util/List;
 */
jobject JNI_FullSplit
(JNIEnv * env, jobject obj, jstring jstr)
{
    /*
    Token result[MIN_TOKEN_BUFSIZE];
    const char * p = env->GetStringUTFChars(jstr, 0);
    int len = mseg_full_split(p, result, MIN_TOKEN_BUFSIZE);
    jobject ret = toJavaTokenList(env,p, result, len);
    env->ReleaseStringUTFChars(jstr, p);
    return ret;
    */
    CALL_MSEG_METHOD(env,jstr,mseg_full_split);
}


/*
 * Class:     com_mingspy_jseg_JSegJNI
 * Method:    UniGramSplit
 * Signature: (Ljava/lang/String;)Ljava/util/List;
 */
jobject JNI_SmartSplit
(JNIEnv * env, jobject obj, jstring jstr)
{
    /*
    Token result[MIN_TOKEN_BUFSIZE];
    const char * p = env->GetStringUTFChars(jstr, 0);
    int len = mseg_smart_split(p, result, MIN_TOKEN_BUFSIZE);
    jobject ret = toJavaTokenList(env,p, result, len);
    env->ReleaseStringUTFChars(jstr, p);
    return ret;
    */
    CALL_MSEG_METHOD(env,jstr,mseg_smart_split);
}

/*
 * Class:     com_mingspy_jseg_JSegJNI
 * Method:    MixSplit
 * Signature: (Ljava/lang/String;)Ljava/util/List;
 */
jobject JNI_Tagging
(JNIEnv * env, jobject obj, jstring jstr)
{
    /*
    Token result[MIN_TOKEN_BUFSIZE];
    const char * p = env->GetStringUTFChars(jstr, 0);
    int len = mseg_tagging(p, result, MIN_TOKEN_BUFSIZE);
    jobject ret = toJavaTokenList(env,p, result, len);
    env->ReleaseStringUTFChars(jstr, p);
    return ret;
    */
    CALL_MSEG_METHOD(env,jstr,mseg_tagging);
}

/*
 * Class:     com_mingspy_mseg_MsegJNI
 * Method:    ConfigSet
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
jobject JNI_ConfigSet
(JNIEnv * env, jobject obj, jstring jkey, jstring jval){
    //
    const char * key = env->GetStringUTFChars(jkey, 0);
    const char * val = env->GetStringUTFChars(jval, 0);
    cerr<<"cpp ->ConfigSet key:"<<key<<"->val:"<<val<<endl;
    mseg_config_set(key, val);
    env->ReleaseStringUTFChars(jkey, key);
    env->ReleaseStringUTFChars(jval, val);
    //return 0;
    // TODO:目前jni函数返回int,bool时抛异常MethodNotFound
    // 所以用jobject代替
    cerr<<"cpp config set key success" <<endl;
    Token result[1];
    return toJavaTokenList(env,"success",result, 1);
}

/*
 * Class:     com_mingspy_mseg_MsegJNI
 * Method:    ConfigInit
 * Signature: (Ljava/lang/String;)I
 */
jobject JNI_ConfigInit
(JNIEnv * env, jobject obj, jstring jstr){
    cerr<<"cpp ->ConfigInit start"<<endl;
    const char * p = env->GetStringUTFChars(jstr, 0);
    mseg_config_init(p);
    env->ReleaseStringUTFChars(jstr, p);
    cerr<<"cpp ->ConfigInit end"<<endl;
    //return 0;
    Token result[1];
    return toJavaTokenList(env,"success",result, 1);
}

/*
 * Class:     com_mingspy_mseg_MsegJNI
 * Method:    MsegInit
 * Signature: ()I
 */
jobject JNI_MsegInit
(JNIEnv *env, jobject obj){
    cerr<<"cpp ->MsegInit start"<<endl;
    mseg_init();
    cerr<<"cpp ->MsegInit end"<<endl;
    //  return (jint)0;
    Token result[1];
    return toJavaTokenList(env,"success",result, 1);
}


static JNINativeMethod s_methods[] = {
    {(char *)"ConfigInit", (char *)"(Ljava/lang/String;)Ljava/util/List;", (void*)&JNI_ConfigInit},
    {(char *)"ConfigSet", (char *)"(Ljava/lang/String;Ljava/lang/String;)Ljava/util/List;", (void*)JNI_ConfigSet},
    {(char *)"MsegInit", (char *)"()Ljava/util/List;", (void*)JNI_MsegInit},
    {(char *)"ForwardSplit", (char *)"(Ljava/lang/String;)Ljava/util/List;", (void*)JNI_ForwardSplit},
    {(char *)"FullSplit", (char *)"(Ljava/lang/String;)Ljava/util/List;", (void*)JNI_FullSplit},
    {(char *)"SmartSplit", (char *)"(Ljava/lang/String;)Ljava/util/List;", (void*)JNI_SmartSplit},
	{(char *)"BackwardSplit", (char *)"(Ljava/lang/String;)Ljava/util/List;", (void*)JNI_BackwardSplit},
    {(char *)"Tagging", (char *)"(Ljava/lang/String;)Ljava/util/List;", (void*)JNI_Tagging}
};

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv* env = NULL;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        perror("cpp: jni get env faild");
        return JNI_ERR;
    }

    jclass cls = env->FindClass("Lcom/mingspy/mseg/MsegJNI;");
    if (cls == NULL) {
        perror("cpp: jni get java class MsegJNI faild");
        return JNI_ERR;
    }

    int len = sizeof(s_methods) / sizeof(s_methods[0]);
    cerr<<"JNI Methods length ="<<len<<endl;
    if (env->RegisterNatives(cls, s_methods, len) < 0) {
        perror( "cpp: jni registerNatives for MsegJNI faild");
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}


