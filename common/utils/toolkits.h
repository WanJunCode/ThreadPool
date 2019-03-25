/**
 * Copyrights (C) 2015 - 2016
 * All rights reserved
 *
 * File: toolkits.h
 *
 * 常用工具类封装
 *
 * Change Logs:
 * Date         Author      Notes
 * 2016-08-04   chenfei     创建
 * ^ 添加rawDataToString(),用于将原始数据块转换成字符串
 */

#ifndef _TOOLKITS_H
#define _TOOLKITS_H

#include <stdint.h>
#include <string>
#include <vector>
#include <string>
#include <sstream>
#include <map>

#define setbit(x,y) x|=(1<<y) //将X的第Y位置1
#define clrbit(x,y) x&=~(1<<y) //将X的第Y位清0

#define BitGet(Number,pos) ((Number) >> (pos)&1) //用宏得到某数的某位


#define ROTATE_LEFT(x, n) ((x) << (n)) | ((x) >> ((8 * sizeof(x)) - (n)))
#define ROTATE_RIGHT(x, n) ((x) >> (n)) | ((x) << ((8 * sizeof(x)) - (n)))


//! 数组元素个数
#define ARRAY_SIZE(array)    (sizeof(array)/sizeof(*(array)))

using std::string;

namespace ToolKits {

/**
 * 字符串分割函数
 * \param src 源字符串
 * \param separator 分隔符
 * \param dest 拆分存储器
 * \return 拆分的字符串数量
 */
size_t split(const std::string& src, const std::string& separator, std::vector<std::string>& dest);
/**
 * 从文件中读取数据
 * \param filename 文件路径
 * \return 数据缓冲区地址
 * \note 用完记得释放;
 *     适用场景：小文件数据读取
 */
char* textFileRead(const char* filename);
/**
 * 向文件写数据
 * \param filename 文件路径
 * \param buf 缓冲区地址
 * \param len 缓冲区长度
 * \return 实际写入文件大小
 */
size_t writeToFile(const char* filename, void* buf, size_t len);
/**
 * 循环移位
 * \param c 源
 * \param b 循环移位数
 * \return 移位后的值
 */
uint8_t crol(uint8_t c, uint8_t b);
uint8_t cror(uint8_t c, uint8_t b);
/**
 * 字符串大小写转换
 * \param str 需转换字符串地址
 * \return 字符串地址
 */
char *strlwr(char *str);
char *strupr(char *str);

/**
 * 将数据块转换成十六进进制的字符串
 */
string rawDataToString(const void *data_ptr, uint16_t data_len, const string &delimiter = string(" "));

/**
 * 将HEX字串转换成数据块
 */
uint16_t stringToRawData(const string &hex_str, void *out_ptr, uint16_t out_len);

/**
 * 常用数据类型转换
 */
template<class DesType, class SrcType>
DesType convert(const SrcType & t) {
    std::stringstream stream;
    stream << t;
    DesType result;
    stream >> result;
    return result;
}

//! 生成随机长度为32B的uuid
string generate_uuid();

/**
 * [字符串替换]
 * \param  str       [源字符串]
 * \param  old_value [旧字符串]
 * \param  new_value [新字符串]
 * \return           [替换后的字符串]
 */
//"12212" 替换["12"]->["21"] 结果["22211"]
string&  replace_all(string& str, const string& old_value, const string& new_value);
//"12212" 替换["12"]->["21"] 结果["21221"]
string&  replace_all_distinct(string& str, const string& old_value, const string& new_value);

// trim from start
string &ltrim(std::string &s);
// trim from end
string &rtrim(std::string &s);
string &trim(std::string &s);

size_t code_convert(const char *from_charset,
                    const char *to_charset, char *inbuf,
                    size_t inlen, char *outbuf, size_t outlen);

//十六进制字符串转换为字节流
int hex2byte(const char* source, int sourceLen, char* dest, int destLen);
int hex2int(const char* source);
int byte2hex(const void *sSrc, int nSrcLen, char *sDest, int destLen);
std::string byteTohex(const void *sSrc, int nSrcLen);

//获取本机MAC及IP地址
bool getLocalInfo(std::pair<std::string, std::string> &ip_addr);
//判断文件夹是否存在
bool IsFolderExist(const char* path);
//可变参数格式化
std::string vform(const char* format, ...);
}

#endif
