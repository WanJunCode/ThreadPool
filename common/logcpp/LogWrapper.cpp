#include "LogWrapper.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <log4cpp/PropertyConfigurator.hh>
#include "../../main/version.h"

extern int errno;

LogWrapper::LogWrapper(void) {
    if (!IsFolderExist(LOG_DIR)) {
        if (0 != mkdir(LOG_DIR, 0766)) {
            fprintf(stderr, "can't creat the directoty about path[%s], error:%s\n , ", LOG_DIR, strerror(errno));
        }
    }

    try {
        log4cpp::PropertyConfigurator::configure(LOG_CONFIG);
    } catch (log4cpp::ConfigureFailure& f) {
        fprintf(stderr, "Configure Problem :%s", f.what());
    }
}

LogWrapper::~LogWrapper(void) {
    log4cpp::Category::shutdown();
}

bool LogWrapper::IsFolderExist(const char* path) {
    DIR *dp = NULL;
    if (NULL == (dp = opendir(path))) {
        return false;
    }

    closedir(dp);
    return true;
}

log4cpp::Category& LogWrapper::GetModuleWrapper(MODULE_TYPE ntype) {
    if (ntype >= 0 && ntype < MODULE_MASTER) {
        return log4cpp::Category::getInstance(categoryName[ntype]);
    }
    return log4cpp::Category::getRoot();
}

log4cpp::CategoryStream LogWrapper::StreamWrapper(MODULE_TYPE ntype, PriorityLevel level) {
    switch (level) {
    case LOG_EMERG:///< system is unusable
        return GetModuleWrapper(ntype).emergStream();
    case LOG_ALERT:///< action must be taken immediately
        return GetModuleWrapper(ntype).alertStream();
    case LOG_CRIT:///< critical conditions
        return GetModuleWrapper(ntype).critStream();
    case LOG_ERROR:///< error conditions
        return GetModuleWrapper(ntype).errorStream();
    case LOG_WARNING:///< warning conditions
        return GetModuleWrapper(ntype).warnStream();
    case LOG_NOTICE:///< normal but significant condition
        return GetModuleWrapper(ntype).noticeStream();
    case LOG_INFO:///< informational
        return GetModuleWrapper(ntype).infoStream();
    default:///< debug-level messages
        return GetModuleWrapper(ntype).debugStream();
    }
}

