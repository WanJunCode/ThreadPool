/**
 * Copyrights (C) 2015 - 2020
 * All right reserved
 *
 * File : util.h
 *
 * 工具函数
 *
 * Change Logs:
 * Date         Author      Notes
 * 2015-08-24   chenfei     Create
 */
#ifndef __CF_UTIL_H__
#define __CF_UTIL_H__


#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
*                               MACRO
**************************************************************************/

//! 安全地释放指针
#define free_safely(ptr) \
    do { \
        if ((ptr) != NULL) { \
            free(ptr); \
            (ptr) = NULL; \
        } \
    } while(0)

#define array_item_number(array)    (sizeof(array)/sizeof(*(array)))

/**
 * 下面的4个宏用于检查代码运行所耗时长用
 */
#define START_L_TIMER(tag) \
    struct timeval _timer_s_##tag; gettimeofday(&_timer_s_##tag, NULL);

#define END_L_TIMER(tag) \
    do { \
        if (timerisset(&_timer_s_##tag)) { \
            struct timeval _timer_e_##tag; \
            gettimeofday(&_timer_e_##tag, NULL); \
            long _run_time = timeval_subtract(&_timer_e_##tag, &_timer_s_##tag); \
            timerclear(&_timer_s_##tag); \
            printf("timer_%s: %ld us\n", #tag, _run_time); \
        } \
    } while (0);

#define DEFINE_G_TIMER(tv) struct timeval tv;
#define START_G_TIMER(tv) gettimeofday(&tv, NULL);
#define END_G_TIMER(tv) \
    do { \
        extern struct timeval tv; \
        if (timerisset(&tv)) { \
            struct timeval end_timeval; \
            gettimeofday(&end_timeval, NULL); \
            long _run_time = timeval_subtract(&end_timeval, &tv); \
            timerclear(&tv); \
            printf("timer_%s: %ld us\n", #tv, _run_time); \
        } \
    } while (0);

#define EXIST_FILE(f) (assert(-1 != (access(f, 0))),f)

/**************************************************************************
*                               FUNCTIONS
**************************************************************************/

/**
 * 生成32个字符的uuid到uuidstr地址所在内存
 * \param   uuidstr 生成uuid的存放内存首地址，内存必须>32bytes
 * \return  char*
 *          - 成功：uuidstr
 *          - 失败：NULL
 */
char* gen_uuid32(char *uuidstr);

/**
 * 生成16个字符的uuid到uuidstr地址所在内存
 * \param   uuidstr 生成uuid的存放内存首地址，内存必须>16bytes
 * \return  char*
 *          - 成功：uuidstr
 *          - 失败：NULL
 */
char* gen_uuid16(char *uuidstr);

/**
 * 将数据转换成可显示的字符串
 * \param buff_ptr  字符串输出缓冲地址
 * \param buff_size 字符串输出缓冲大小
 * \param data_ptr  数据块地址
 * \param data_len  数据块大小
 */
const char* rawdata_to_str(char *buff_ptr, size_t buff_size, const void *data_ptr, size_t data_len);

/**
 * 时间戳相减，返回以微秒为单位的时间间隔
 *
 * \param lhs 时间戳1
 * \param rhs 时间戳2，lhs应该大于rhs
 *
 * \return long 时间间隔
 */
long timeval_subtract(const struct timeval *lhs, const struct timeval *rhs);

/**
 * 从指定file_name文件中读取所有数据，并返回缓冲buffer
 * \param file_name 文件名
 * \return char* 文件数据缓冲
 * \note 返回的buffer必须要释放
 */
char* ReadFile(const char *file_name);

/**
 * 生成随机字符串
 * \param   buff
 * \param   size
 * \node buff的大小必须为size，自动加结束符
 */
void generate_rand_key(char *buff, size_t size);

/**
 * 生成指定个数的0~9随机数字
 * \param ptr 缓冲区地址，空间必须大于len
 * \param len 随机数据的个数
 * \return char * 返回ptr
 */
char* GenerateRandNumbers(char *ptr, int len);

/**
 * 字符串相等比较
 */
bool StringEqual(const char *lhs, const char *rhs);

/**
 * 设置系统时间
 */
int SetSystemTime(time_t setime);

/**
 * 设置系统时区
 */
int SetTimezone(int tz_offset);

/**
 * 根据'YYYY-MM-DD HH:MM:SS'生成time_t
 */
time_t MakeTimeByString(const char *time_str);

/**
 * 当前线程id
 */
pid_t CurrentThreadId();

/**
 * 计算字符串的Hash值
 */
uint32_t CalcStringHash(const char *str);

//! 生成随机数
int GenerateRandom();

//! 获取UTC时间戳
time_t UnixTime();

//! utc 时间戳转换成本地时间
void UnixTime2String(time_t n, char* buff_ptr, int buff_len);

//! 获取CPU数量
int GetCpuNum();

/**
 * Helper function to increase the max file descriptors limit
 * for the current process and all of its children.
 * By default, tries to increase it to as much as 2^24.
 */
int IncreaseMaxFds(int max_fds);

#ifdef __cplusplus
}
#endif

#endif
