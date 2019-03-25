#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>
#include <uuid/uuid.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include "util.h"

/**************************************************************************
*                                FUNCTIONS
**************************************************************************/

/**
 * 调用uuid库接口生成32个字符的uuid到uuidstr地址所在内存
 */
char* gen_uuid32(char *uuidstr) {
    uuid_t uuid ;

    uuid_generate(uuid);
    if (uuid_is_null(uuid))
        return NULL;

    char uuid_str[37] = {0};
    uuid_unparse(uuid, uuid_str);

    //to cut off "-"
    strncpy(uuidstr, uuid_str, 8);
    strncpy(&uuidstr[8], &uuid_str[9], 4);
    strncpy(&uuidstr[12], &uuid_str[14], 4);
    strncpy(&uuidstr[16], &uuid_str[19], 4);
    strncpy(&uuidstr[20], &uuid_str[24], 12);

    return uuidstr;
}

/**
 * 调用uuid库接口生成16个字符的uuid到uuidstr地址所在内存
 */
char* gen_uuid16(char *uuidstr) {
    uuid_t uuid ;
    char uuid_str[37] = {0};

    uuid_generate(uuid);
    if (uuid_is_null(uuid))
        return NULL;

    uuid_unparse(uuid, uuid_str);
    //to cut off "-"
    strncpy(uuidstr, uuid_str, 8);
    strncpy(&uuidstr[8], &uuid_str[9], 4);
    strncpy(&uuidstr[12], &uuid_str[14], 4);

    return uuidstr;
}

const char* rawdata_to_str(char *buff_ptr, size_t buff_size, const void *data_ptr, size_t data_len) {
    buff_ptr[0] = '\0';
    if (data_ptr == NULL || data_len == 0)
        return NULL;

    char tmp[5] = {0};

    for (size_t i = 0; i < data_len; ++i) {
        const char *fmt = "%02X ";
        if (i == data_len - 1) //! 最后一个不加空格
            fmt = "%02X";

        snprintf(tmp, sizeof(tmp), fmt, ((const uint8_t*)data_ptr)[i]);
        strncat(buff_ptr, tmp, buff_size);
    }
    buff_ptr[buff_size - 1] = '\0';
    return buff_ptr;
}

/**
 * 两个timeval相减，得出微秒值
 */
long timeval_subtract(const struct timeval *lhs, const struct timeval *rhs) {
    long sec_sub = lhs->tv_sec - rhs->tv_sec;
    long usec_sub = lhs->tv_usec - rhs->tv_usec;
    return sec_sub * 1000000 + usec_sub;
}

/**
 * 从指定file_name文件中读取所有数据，并返回缓冲buffer
 * \param file_name 文件名
 * \return char* 文件数据缓冲
 * \note 返回的buffer必须要释放
 */
char* ReadFile(const char *file_name) {
    FILE *file = fopen(file_name, "r");
    char *buffer = NULL;
    if (file != NULL) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);
        buffer = (char*)malloc(file_size);
        if (buffer != NULL) {
            fread(buffer, file_size, 1, file);
        }
        fclose(file);
    }
    return buffer;
}

/**
 * 生成随机字符串
 * \param   buff
 * \param   size
 * \node buff的大小必须为size，自动加结束符
 */
void generate_rand_key(char *buff, size_t size) {
    srand(time(NULL));
    const char *characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (size_t i = 0; i < size - 1; ++i) {
        int index = rand() % 62;
        buff[i] = characters[index];
    }
    buff[size - 1] = '\0';
}

char* GenerateRandNumbers(char *ptr, int len) {
    srand(time(NULL));
    for (int i = 0; i < len; ++i)
        ptr[i] = '0' + (rand() % 10);
    ptr[len] = '\0';
    return ptr;
}

bool StringEqual(const char *lhs, const char *rhs) {
    return (strcmp(lhs, rhs) == 0);
}

int SetSystemTime(time_t setime) {
    struct timeval tv;
    tv.tv_sec = setime;
    tv.tv_usec = 0;

    if (settimeofday (&tv, (struct timezone *) 0) < 0) {
        printf("Set system datatime error!/n");
        return -1;
    }

    return 0;
}

/**
 * 设置系统时区
 * \param tz_offset 时区偏移，单位是second
 *                  > 0，东区
 *                  < 0，西区
 */
int SetTimezone(int tz_offset) {
    char west_or_east = '+';
    if (tz_offset > 0)
        west_or_east = '-';

    unsigned int u_tz_offset = abs(tz_offset);
    unsigned int offset_hours = u_tz_offset / 3600;
    u_tz_offset %= 3600;
    unsigned int offset_minutes = u_tz_offset / 60;
    u_tz_offset %= 60;
    unsigned int offset_seconds = u_tz_offset;

    char buff[50] = {0};
    snprintf(buff, sizeof(buff), "set_timezone 'GMT%c%ul:%ul:%ul'",
             west_or_east, offset_hours, offset_minutes, offset_seconds);

    return system(buff);
}

//! 将YYYY-MM-DD HH:MM:SS转成time_t
time_t MakeTimeByString(const char *time_str) {
    if (time_str == NULL)
        return 0;

    int year = 0, month = 0, day = 0;
    int hour = 0, minute = 0, second = 0;
    int ret = sscanf(time_str, "%d-%d-%d %d:%d:%d",
                     &year, &month, &day, &hour, &minute, &second);
    if (ret < 6)
        return 0;

    struct tm tmp;
    tmp.tm_year = year - 1900;
    tmp.tm_mon = month - 1;
    tmp.tm_mday = day;
    tmp.tm_hour = hour;
    tmp.tm_min = minute;
    tmp.tm_sec = second;

    return mktime(&tmp);
}

pid_t CurrentThreadId() {
    return (pid_t)(syscall(SYS_gettid));
}

//! 计算字符串的HASH值，RSHash算法
uint32_t CalcStringHash(const char *str) {
    uint32_t b = 378551, a = 63689;
    uint32_t hash = 0;

    while (*str) {
        hash = hash * a + (*str++);
        a *= b;
    }

    return (hash & 0x7FFFFFFF);
}

//! 生成随机数
int GenerateRandom() {
    static unsigned int count = 0;
    srand((unsigned)time(NULL) + count);
    ++count;
    return rand();
}

time_t UnixTime() {
    return time(NULL);
}

void UnixTime2String(time_t n, char* buff_ptr, int buff_len) {
    struct tm stm_;
    localtime_r(&n, &stm_);
    strftime(buff_ptr, buff_len - 1, "%Y-%m-%d %H:%M:%S", &stm_);
    buff_ptr[buff_len - 1] = '\0';
}

int GetCpuNum() {
    char buf[16] = {0x00};
    int num;
    FILE* fp = popen("cat /proc/cpuinfo |grep processor|wc -l", "r");
    if (fp) {
        fread(buf, 1, sizeof(buf) - 1, fp);
        pclose(fp);
    }
    num = atoi(buf);
    if (num <= 0)
        num = 1;

    return num;
}

int IncreaseMaxFds(int max_fds) {
    if (-1 == max_fds)
        max_fds = (1 << 24);

    struct rlimit fdmaxrl;
    for (fdmaxrl.rlim_cur = max_fds, fdmaxrl.rlim_max = max_fds;
            max_fds && (setrlimit(RLIMIT_NOFILE, &fdmaxrl) < 0);
            fdmaxrl.rlim_cur = max_fds, fdmaxrl.rlim_max = max_fds) {
        max_fds /= 2;
    }

    return (int)(fdmaxrl.rlim_cur);
}