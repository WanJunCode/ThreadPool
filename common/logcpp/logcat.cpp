#include "logcat.h"
#include "netsrv.h"
#include <stdio.h>
#include <memory.h>
#include <sys/time.h>
#include "LogWrapper.h"

static const std::string priority_names[10] = {
    "[EMERG]",
    "[ALERT]",
    "[CRIT]",
    "[ERROR]",
    "[WARNING]",
    "[NOTICE]",
    "[INFO]",
    "[DEBUG]",
    "[NOTSET]",
    "[UNKNOWN]"
};

namespace cpplog {
Category& root = Category::getInstance();
}

Category Category::_category;

Category& Category::getInstance() {
    return _category;
}

Category::Category(PriorityLevel priority)
    : _priority(priority)
    , _server(NULL) {
    if (NULL == _server) {
        //! FIXME: 这里打开后会导致程序在MiniHub上跑不起来
        //_server = new NetServer();
    }
#ifdef SYSLOG
    openlog("EdgeServer", LOG_CONS | LOG_PID, LOG_LOCAL2);
#endif
}

Category::~Category() {
    if (NULL == _server)
        delete _server;
    _server = NULL;
    closelog();
}


void Category::startup(const char* ip, unsigned short port) {
    if (NULL != _server) {
        _server->startup(ip, port);
    }
}

void Category::shutdown() {
    if (NULL != _server) {
        _server->shutdown();
    }
}

void Category::_logUnconditionally(PriorityLevel priority, const char* format, va_list arguments) {
    _logUnconditionally2(priority, vform(format, arguments));
}

std::string Category::vform(const char* format, va_list args) {
    size_t size = 1024;
    char* buffer = new char[size];

    while (1) {
        va_list args_copy;
        va_copy(args_copy, args);

        int n = vsnprintf(buffer, size, format, args_copy);//function error,replace it

        va_end(args_copy);

        // If that worked, return a string.
        if ((n > -1) && (static_cast<size_t>(n) < size)) {
            std::string s(buffer);
            delete [] buffer;
            return s;
        }

        // Else try again with more space.
        size = (n > -1) ?
               n + 1 :   // ISO/IEC 9899:1999
               size * 2; // twice the old size

        delete [] buffer;
        buffer = new char[size];
    }
}

std::string GetSystemTime()  {
    char time_str[25] = {0};
#ifdef WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(time_str, sizeof(timer_str),
             "[%04d-%02d-%02d %02d:%02d:%02d.%03ld]",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    struct tm *p = localtime(&tv.tv_sec);
    snprintf(time_str, sizeof(time_str),
             "[%04d-%02d-%02d %02d:%02d:%02d.%03ld]",
             1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
             p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec / 1000);
    time_str[sizeof(time_str) - 1] = '\0';  //! 确保有结尾符
#endif
    return time_str;
}

void Category::_logUnconditionally2(PriorityLevel priority, const std::string& message) {

#if SYSLOG
    std::string logmsg = priority_names[priority] + message;
//发送网络数据
    if (NULL != _server) {
        _server->send(logmsg.c_str(), logmsg.length());
    }

    syslog(priority, "%s", logmsg.c_str());
#elif LOG4CPP
    LogWrapper::Instance().StreamWrapper(MODULE_MASTER, priority) << message;
#else
    std::string logmsg = GetSystemTime() + priority_names[priority] + message;
    //发送网络数据
    if (NULL != _server) {
        _server->send(logmsg.c_str(), logmsg.length());
    }

    std::cout << logmsg << std::endl;
    std::cout.flush();
#endif
}

PriorityLevel Category::getPriority() const {
    return _priority;
}

void Category::setPriority(PriorityLevel priority) {
    if ((priority < LOG_NOTSET)) {
        _priority = priority;
    }
}

bool Category::isPriorityEnabled(PriorityLevel priority) const {
    return (priority <= _priority);
}

void Category::log(PriorityLevel priority, const char* stringFormat, ...) {
    if (isPriorityEnabled(priority)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(priority, stringFormat, va);
        va_end(va);
    }
}

void Category::log(PriorityLevel priority, const std::string& message) {
    if (isPriorityEnabled(priority)) {
        _logUnconditionally2(priority, message);
    }
}

CategoryStream Category::log(PriorityLevel priority) {
    using cpplog::root;
    switch (priority) {
    case LOG_EMERG:
        return root.emerg();
    case LOG_ALERT:
        return root.alert();
    case LOG_CRIT:
        return root.crit();
    case LOG_ERR:
        return root.error();
    case LOG_WARNING:
        return root.warn();
    case LOG_NOTICE:
        return root.notice();
    case LOG_INFO:
        return root.info();
    case LOG_DEBUG:
        return root.debug();
        break;
    default:
        break;
    }
    return root.notset();
}

void Category::debug(const char* stringFormat, ...) {
    if (isPriorityEnabled(LOG_DEBUG)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(LOG_DEBUG, stringFormat, va);
        va_end(va);
    }
}

void Category::debug(const std::string& message) {
    if (isPriorityEnabled(LOG_DEBUG))
        _logUnconditionally2(LOG_DEBUG, message);
}

CategoryStream Category::debug() {
    return getStream(LOG_DEBUG);
}

void Category::info(const char* stringFormat, ...) {
    if (isPriorityEnabled(LOG_INFO)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(LOG_INFO, stringFormat, va);
        va_end(va);
    }
}

void Category::info(const std::string& message) {
    if (isPriorityEnabled(LOG_INFO))
        _logUnconditionally2(LOG_INFO, message);
}

CategoryStream Category::info() {
    return getStream(LOG_INFO);
}

void Category::notice(const char* stringFormat, ...) {
    if (isPriorityEnabled(LOG_NOTICE)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(LOG_NOTICE, stringFormat, va);
        va_end(va);
    }
}

void Category::notice(const std::string& message) {
    if (isPriorityEnabled(LOG_NOTICE))
        _logUnconditionally2(LOG_NOTICE, message);
}

CategoryStream Category::notice() {
    return getStream(LOG_NOTICE);
}

void Category::warn(const char* stringFormat, ...) {
    if (isPriorityEnabled(LOG_WARNING)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(LOG_WARNING, stringFormat, va);
        va_end(va);
    }
}

void Category::warn(const std::string& message) {
    if (isPriorityEnabled(LOG_WARNING))
        _logUnconditionally2(LOG_WARNING, message);
}

CategoryStream Category::warn() {
    return getStream(LOG_WARNING);
}


void Category::error(const char* stringFormat, ...) {
    if (isPriorityEnabled(LOG_ERR)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(LOG_ERR, stringFormat, va);
        va_end(va);
    }
}

void Category::error(const std::string& message) {
    if (isPriorityEnabled(LOG_ERR))
        _logUnconditionally2(LOG_ERR, message);
}

CategoryStream Category::error() {
    return getStream(LOG_ERR);
}

void Category::crit(const char* stringFormat, ...) {
    if (isPriorityEnabled(LOG_CRIT)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(LOG_CRIT, stringFormat, va);
        va_end(va);
    }
}

void Category::crit(const std::string& message) {
    if (isPriorityEnabled(LOG_CRIT))
        _logUnconditionally2(LOG_CRIT, message);
}

CategoryStream Category::crit() {
    return getStream(LOG_CRIT);
}

void Category::alert(const char* stringFormat, ...) {
    if (isPriorityEnabled(LOG_ALERT)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(LOG_ALERT, stringFormat, va);
        va_end(va);
    }
}

void Category::alert(const std::string& message) {
    if (isPriorityEnabled(LOG_ALERT))
        _logUnconditionally2(LOG_ALERT, message);
}

CategoryStream Category::alert() {
    return getStream(LOG_ALERT);
}

void Category::emerg(const char* stringFormat, ...) {
    if (isPriorityEnabled(LOG_EMERG)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(LOG_EMERG, stringFormat, va);
        va_end(va);
    }
}

void Category::emerg(const std::string& message) {
    if (isPriorityEnabled(LOG_EMERG))
        _logUnconditionally2(LOG_EMERG, message);
}

CategoryStream Category::emerg() {
    return getStream(LOG_EMERG);
}

void Category::notset(const char* stringFormat, ...) {
    if (isPriorityEnabled(LOG_NOTSET)) {
        va_list va;
        va_start(va, stringFormat);
        _logUnconditionally(LOG_NOTSET, stringFormat, va);
        va_end(va);
    }
}

void Category::notset(const std::string& message) {
    if (isPriorityEnabled(LOG_NOTSET))
        _logUnconditionally2(LOG_NOTSET, message);
}

CategoryStream Category::notset() {
    return getStream(LOG_NOTSET);
}

CategoryStream Category::getStream(PriorityLevel priority) {
    return CategoryStream(*this, isPriorityEnabled(priority) ? priority : LOG_NOTSET);
}

//////////////////////////////////////////////////////////////////////////

CategoryStream::CategoryStream(Category& category, PriorityLevel priority) :
    _category(category),
    _priority(priority),
    _buffer(NULL) {
}

CategoryStream::~CategoryStream() {
    flush();
}

void CategoryStream::flush() {
    if (_buffer) {
        getCategory().log(_priority, _buffer->str());
        delete _buffer;
        _buffer = NULL;
    }
}

CategoryStream& CategoryStream::operator<<(const char* t) {
    if (getPriority() <= LOG_NOTSET) {
        if (!_buffer) {
            if (!(_buffer = new std::ostringstream)) {
                // XXX help help help
            }
        }
        (*_buffer) << t;
    }
    return *this;
}

CategoryStream& CategoryStream::operator<< (cspf pf) {
    return (*pf)(*this);
}

CategoryStream& eol (CategoryStream& os) {
    if  (os._buffer) {
        os.flush();
    }
    return os;
}
CategoryStream& left (CategoryStream& os) {
    if  (os._buffer) {
        os._buffer->setf(std::ios::left);
    }
    return os;
}
