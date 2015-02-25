#include <iostream>

#include "knife.hpp"

#ifdef __cplusplus
extern "C" {
#endif

const int MIN_TOKEN_BUFSIZE = 10000;

/*
 * 指定配置配置文件初始化
 */
void mseg_config_init(const char * config_path);

void mseg_config_set(const char * key, const char * val);

void mseg_init();

/*
 * 使用前向切分
 * 分词结果保存在result数组中, 返回结果的时间长度。
 * @param str : 待切分字符串,utf-8编码格式的
 * @param result : 切分的结果数组，要求提前分配好空间。
 * @result_len : 切分结果数组长度,参考 MIN_TOKEN_BUFSIZE
 * @ return :  返回实际切分长度
 */
int mseg_forward_split(const char * str, struct Token * result, int result_len); 

/*
 * 使用后向切分
 * 分词结果保存在result数组中, 返回结果的时间长度。
 * @param str : 待切分字符串,utf-8编码格式的
 * @param result : 切分的结果数组，要求提前分配好空间。
 * @result_len : 切分结果数组长度,参考 MIN_TOKEN_BUFSIZE
 * @ return :  返回实际切分长度
 */
int mseg_backward_split(const char * str, struct Token * result, int result_len); 

/*
 * 使用智能切分
 * 分词结果保存在result数组中, 返回结果的时间长度。
 * @param str : 待切分字符串,utf-8编码格式的
 * @param result : 切分的结果数组，要求提前分配好空间。
 * @result_len : 切分结果数组长度,参考 MIN_TOKEN_BUFSIZE
 * @ return :  返回实际切分长度
 */
int mseg_smart_split(const char * str, struct Token * result, int result_len); 

/*
 * 使用全切分
 * 分词结果保存在result数组中, 返回结果的时间长度。
 * @param str : 待切分字符串,utf-8编码格式的
 * @param result : 切分的结果数组，要求提前分配好空间。
 * @result_len : 切分结果数组长度,参考 MIN_TOKEN_BUFSIZE
 * @ return :  返回实际切分长度
 */
int mseg_full_split(const char * str, struct Token * result, int result_len); 

/*
 * 标注分词结果 
 * 分词结果保存在result数组中, 返回结果的时间长度。
 * @param str : 待切分字符串,utf-8编码格式的
 * @param result : 切分的结果数组，要求提前分配好空间。
 * @result_len : 切分结果数组长度
 * @ return :  返回实际切分长度
 */
int mseg_tagging(const char * str, struct Token * result, int result_len); 

const char * mseg_get_pos(int id);

#ifdef __cplusplus
}
#endif
