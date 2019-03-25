#include "toolkits.h"
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <stdarg.h>
#include <algorithm>
#include <uuid/uuid.h>
#include <iconv.h>
#include <memory>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>

namespace ToolKits {

size_t split(const std::string& src, const std::string& separator, std::vector<std::string>& dest) {
    std::string str = src;
    std::string substring;
    std::string::size_type start = 0, index;
    size_t len = dest.size();
    do {
        index = str.find_first_of(separator, start);
        if (index != std::string::npos) {
            substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = str.find_first_not_of(separator, index);
            if (start == std::string::npos)
                return dest.size() - len;
        }
    } while (index != std::string::npos);

    //the last token
    substring = str.substr(start);
    if (!substring.empty())
        dest.push_back(substring);
    return dest.size() - len;
}

char* textFileRead(const char* filename) {
    FILE *pf = fopen(filename, "r");
    if (pf != NULL) {
        fseek(pf, 0, SEEK_END);
        long lSize = ftell(pf);
        rewind(pf);
        char* pText = (char*)malloc(lSize + 1);
        if (NULL != pText) {
            if (0 < fread(pText, sizeof(char), lSize, pf)) {
                pText[lSize] = '\0';
            }
        }
        fclose(pf);
        return pText;
    }
    return NULL;
}

//向文件写数据
size_t writeToFile(const char* filename, void* buf, size_t len) {
    FILE* fp = fopen(filename, "w");
    if (fp) {
        size_t slen = fwrite(buf, len, 1, fp);
        fclose(fp);
        return slen;
    }
    return 0;
}

uint8_t crol(uint8_t c, uint8_t b) {
    uint8_t left = c << b;
    uint8_t right = c >> (8 - b);
    uint8_t temp = left | right;
    return temp;
}

uint8_t cror(uint8_t c, uint8_t b) {
    uint8_t right = c >> b;
    uint8_t left = c << (8 - b);
    uint8_t temp = left | right;
    return temp;
}

char * strlwr(char *str) {
    char *orign = str;
    for (; *str != '\0'; str++)
        *str = tolower(*str);
    return orign;
}

char * strupr(char *str) {
    char *orign = str;
    for (; *str != '\0'; str++)
        *str = toupper(*str);
    return orign;
}

// trim from start
string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

string rawDataToString(const void *data_ptr, uint16_t data_len, const string &delimiter) {
    if (data_ptr == NULL || data_len == 0)
        return string();

    using namespace std;
    ostringstream oss;
    oss << hex << setfill('0');
    const uint8_t *ptr = static_cast<const uint8_t*>(data_ptr);
    for (uint16_t i = 0; i < data_len; ++i) {
        oss << setw(2) << (int)ptr[i];
        if (i < (data_len - 1))
            oss << delimiter;
    }
    return oss.str();
}

static uint8_t hexCharToValue(char hex_char) {
    if ('0' <= hex_char && hex_char <= '9')
        return hex_char - '0';
    else if ('A' <= hex_char && hex_char <= 'F')
        return hex_char - 'A' + 10;
    else if ('a' <= hex_char && hex_char <= 'f')
        return hex_char - 'a' + 10;
    else
        return 0;
}

uint16_t stringToRawData(const string &hex_str, void *out_ptr, uint16_t out_len) {
    if (out_ptr == NULL || out_len == 0)
        return 0;

    uint8_t *p_data = (uint8_t*)out_ptr;
    uint16_t data_len = 0;
    for (uint16_t i = 0; (i < out_len) && ((i * 2 + 1) < (uint16_t)hex_str.size()); ++i) {
        char h_char = hex_str[2 * i];
        char l_char = hex_str[2 * i + 1];
        p_data[i] = (hexCharToValue(h_char) << 4) | (hexCharToValue(l_char) & 0x0f);
        ++ data_len;
    }
    return data_len;
}

string generate_uuid() {
    uuid_t uuid ;

    uuid_generate(uuid);
    if (uuid_is_null(uuid))
        return "";

    char uuid_str[37] = {0};
    uuid_unparse(uuid, uuid_str);

    //to cut off "-"
    char uuid32str[33] = {0};
    memcpy(uuid32str, uuid_str, 8);
    memcpy(&uuid32str[8], &uuid_str[9], 4);
    memcpy(&uuid32str[12], &uuid_str[14], 4);
    memcpy(&uuid32str[16], &uuid_str[19], 4);
    memcpy(&uuid32str[20], &uuid_str[24], 12);
    uuid32str[32] = '\0';

    return uuid32str;
}

string& replace_all(string& str, const string& old_value, const string& new_value) {
    while (true)   {
        string::size_type   pos(0);
        if ((pos = str.find(old_value)) != string::npos   )
            str.replace(pos, old_value.length(), new_value);
        else
            break;
    }
    return str;
}

string& replace_all_distinct(string& str, const string& old_value, const string& new_value) {
    for (string::size_type   pos(0); pos != string::npos; pos += new_value.length())   {
        if ((pos = str.find(old_value, pos)) != string::npos )
            str.replace(pos, old_value.length(), new_value);
        else
            break;
    }
    return str;
}

size_t code_convert(const char *from_charset, const char *to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
    char **pin = &inbuf;
    char **pout = &outbuf;

    iconv_t cd = iconv_open(to_charset, from_charset);
    if (0 != cd) {
        memset(outbuf, 0, outlen);
        size_t outbytesleft = outlen;
        iconv(cd, pin, &inlen, pout, &outbytesleft);
        iconv_close(cd);
        return outlen - outbytesleft;
    }
    return 0;
}

///十六进制字符串转换为字节流
int hex2byte(const char* source, int sourceLen, char* dest, int destLen) {
    if (sourceLen / 2 > destLen)
        return 0;

    unsigned char highByte, lowByte;
    for (int i = 0; i < sourceLen; i += 2) {
        highByte = toupper(source[i]);
        lowByte = toupper(source[i + 1]);

        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;

        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;

        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return sourceLen / 2;
}

//字节流转换为十六进制字符串的另一种实现方式
int byte2hex(const void *sSrc, int nSrcLen, char *sDest, int destLen) {
    if (nSrcLen * 2 > destLen)
        return 0;

    char szTmp[3] = { 0x00 };
    char *ptr = (char *)sSrc;
    for (int i = 0; i < nSrcLen; i++) {
        sprintf(szTmp, "%02X", (unsigned char)ptr[i]);
        memcpy(&sDest[i * 2], szTmp, 2);
    }
    return nSrcLen * 2;
}

std::string byteTohex(const void *sSrc, int nSrcLen) {
    int len = nSrcLen * 2 + 1;
    std::unique_ptr<char> dst(new char[len]);
    memset(dst.get(),0x00,len);
    if (byte2hex(sSrc, nSrcLen, dst.get(), len)) {
        return dst.get();
    }
    return "";
}

int hex2int(const char* s) {
    int i;
    int n = 0;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        i = 2;
    } else {
        i = 0;
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i) {
        if (tolower(s[i]) > '9') {
            n = 16 * n + (10 + tolower(s[i]) - 'a');
        } else {
            n = 16 * n + (tolower(s[i]) - '0');
        }
    }
    return n;
}

#define MAXINTERFACES   16
bool getLocalInfo(std::pair<std::string, std::string> &ip_addr) {
    register int fd, intrface, retn = 0;
    struct ifreq buf[MAXINTERFACES];
    struct ifconf ifc;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)) {
            intrface = ifc.ifc_len / sizeof(struct ifreq);
            while (intrface-- > 0) {
                //判断网卡类型
                if (!(ioctl(fd, SIOCGIFFLAGS, (char *)&buf[intrface]))) {
                    if (buf[intrface].ifr_flags & IFF_PROMISC) {
                        puts("the interface is PROMISC");
                        retn++;
                    }
                }
                //获取当前网卡的IP地址
                if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface]))) {
                    ip_addr.first = ::inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr);
                }
                /* this section can't get Hardware Address,I don't know whether the reason is module driver*/
                if (!(ioctl(fd, SIOCGIFHWADDR, (char *)&buf[intrface]))) {
                    char mac[256] = { 0x00 };
                    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x\n",
                            (unsigned char)buf[intrface].ifr_hwaddr.sa_data[0],
                            (unsigned char)buf[intrface].ifr_hwaddr.sa_data[1],
                            (unsigned char)buf[intrface].ifr_hwaddr.sa_data[2],
                            (unsigned char)buf[intrface].ifr_hwaddr.sa_data[3],
                            (unsigned char)buf[intrface].ifr_hwaddr.sa_data[4],
                            (unsigned char)buf[intrface].ifr_hwaddr.sa_data[5]);
                    ip_addr.second = mac;
                    return true;
                }
            } //while
        }
        close(fd);
    }
    return false;
}

bool IsFolderExist(const char* path) {
    DIR *dp = NULL;
    if (NULL == (dp = opendir(path))) {
        return false;
    }

    closedir(dp);
    return true;
}

std::string vform(const char* format, ...) {
    va_list args;
    va_start(args, format);

    size_t size = 1024;
    char* buffer = new char[size];

    while (1) {
        memset(buffer, 0x00, size);
        va_list args_copy;
        va_copy(args_copy, args);

        int n = vsnprintf(buffer, size, format, args_copy);//function error,replace it

        va_end(args_copy);

        // If that worked, return a string.
        if ((n > -1) && (static_cast<size_t>(n) < size)) {
            std::string s(buffer, n);
            delete[] buffer;
            va_end(args);
            return s;
        }

        // Else try again with more space.
        size = (n > -1) ?
               n + 1 :   // ISO/IEC 9899:1999
               size * 2; // twice the old size

        delete[] buffer;
        buffer = new char[size];
    }
}

}
