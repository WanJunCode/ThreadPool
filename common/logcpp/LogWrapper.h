#ifndef __CF_LOG_WRAPPER_H
#define __CF_LOG_WRAPPER_H

#include <log4cpp/Category.hh>
#include <log4cpp/CategoryStream.hh>
#include "module.h"
#include "logcat.h"

class LogWrapper {
public:
    static LogWrapper &Instance() {
        static LogWrapper log_;
        return log_;
    }
    ~LogWrapper(void);
public:
    log4cpp::Category& GetModuleWrapper(MODULE_TYPE ntype);
    log4cpp::CategoryStream StreamWrapper(MODULE_TYPE ntype, PriorityLevel level);
protected:
    LogWrapper();
    LogWrapper(LogWrapper &) = delete;
    LogWrapper & operator=(const LogWrapper &) = delete;
private:
    bool IsFolderExist(const char* path);
};
#endif