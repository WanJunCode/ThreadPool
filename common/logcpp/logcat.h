#ifndef LOGCAT_H_20160901
#define LOGCAT_H_20160901

#include <stdarg.h>
#include <syslog.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <ios>
#include <sstream>
#include <stdexcept>

/// from syslog.h
// #define LOG_EMERG   0       ///< system is unusable
// #define LOG_ALERT   1      ///< action must be taken immediately
// #define LOG_CRIT    2      ///< critical conditions
// #define LOG_ERROR   3      ///< error conditions
// #define LOG_WARNING 4       ///< warning conditions
// #define LOG_NOTICE  5       ///< normal but significant condition
// #define LOG_INFO    6       ///< informational
// #define LOG_DEBUG   7       ///< debug-level messages

#define LOG_ERROR LOG_ERR

typedef int PriorityLevel;
#define LOG_NOTSET (LOG_DEBUG + 1)

class Category;
class CategoryStream {
public:
    CategoryStream(Category& category, PriorityLevel priority);
    ~CategoryStream(void);
public:
    /**
         * Flush the contents of the stream buffer to the Category and
         * empties the buffer.
         **/
    void flush();

    /**
         * Stream in arbitrary types and objects.
         * @param t The value or object to stream in.
         * @returns A reference to itself.
         **/
    template<typename T>
    CategoryStream& operator<<(const T& t) {
        if (getPriority() < LOG_NOTSET) {
            if (!_buffer) {
                if (!(_buffer = new std::ostringstream)) {
                    // XXX help help help
                }
            }
            (*_buffer) << t;
        }
        return *this;
    }

    CategoryStream& operator<<(const char* t);

    template<typename T>
    CategoryStream& operator<<(const std::string& t) {
        if (getPriority() < LOG_NOTSET) {
            if (!_buffer) {
                if (!(_buffer = new std::ostringstream)) {
                    // XXX help help help
                }
            }
            (*_buffer) << t;
        }
        return *this;
    }
    /**
    * Returns the destination Category for this stream.
    * @returns The Category.
    **/
    inline Category& getCategory() const {
        return _category;
    };

    /**
     * Returns the priority for this stream.
     * @returns The priority.
     **/
    inline PriorityLevel getPriority() const  {
        return _priority;
    };
    /**
         * Set the width output on CategoryStream
         **/
    std::streamsize width(std::streamsize wide );

public:
    typedef CategoryStream& (*cspf) (CategoryStream&);

    CategoryStream& operator << (cspf);
    friend CategoryStream& eol(CategoryStream& os);
    friend CategoryStream& left(CategoryStream& os);

private:
    Category& _category;
    PriorityLevel _priority;
    std::ostringstream* _buffer;
};

CategoryStream& eol(CategoryStream& os);
CategoryStream& left(CategoryStream& os);

class NetServer;
class Category {
public:
    /**
     * Instantiate a Category with name <code>name</code>. This
     * method does not set priority of the category which is by
     * default <code>Priority::NOTSET</code>.
     *
     * @param name The name of the category to retrieve.
     **/
    static Category& getInstance();

    /**
     * This method will start local and network server
     **/
    void startup(const char* ip, unsigned short port);

    /**
     * This method will close log server
     **/
    void shutdown();

    /**
     * Destructor for Category.
     **/
    virtual ~Category();

    /**
         * Log a message with the specified priority.
         * @param priority The priority of this log message.
         * @param stringFormat Format specifier for the string to write
         * in the log file.
         * @param ... The arguments for stringFormat
         **/
    virtual void log(PriorityLevel priority, const char* stringFormat, ...) ;
    virtual void log(PriorityLevel priority, const std::string& message);
    CategoryStream log(PriorityLevel priority);

    /**
     * Log a message with debug priority.
     * @param stringFormat Format specifier for the string to write
     * in the log file.
     * @param ... The arguments for stringFormat
     **/
    void debug(const char* stringFormat, ...);
    void debug(const std::string& message);
    CategoryStream debug();


    /**
     * Log a message with info priority.
     * @param stringFormat Format specifier for the string to write
     * in the log file.
     * @param ... The arguments for stringFormat
     **/
    void info(const char* stringFormat, ...) ;
    void info(const std::string& message) ;
    CategoryStream info();

    /**
     * Log a message with notice priority.
     * @param stringFormat Format specifier for the string to write
     * in the log file.
     * @param ... The arguments for stringFormat
     **/
    void notice(const char* stringFormat, ...) ;
    void notice(const std::string& message) ;
    CategoryStream notice();

    /**
     * Log a message with warn priority.
     * @param stringFormat Format specifier for the string to write
     * in the log file.
     * @param ... The arguments for stringFormat
     **/
    void warn(const char* stringFormat, ...) ;
    void warn(const std::string& message) ;
    CategoryStream warn();

    /**
     * Log a message with error priority.
     * @param stringFormat Format specifier for the string to write
     * in the log file.
     * @param ... The arguments for stringFormat
     **/
    void error(const char* stringFormat, ...) ;
    void error(const std::string& message) ;
    CategoryStream error();

    /**
     * Log a message with crit priority.
     * @param stringFormat Format specifier for the string to write
     * in the log file.
     * @param ... The arguments for stringFormat
     **/
    void crit(const char* stringFormat, ...) ;
    void crit(const std::string& message) ;
    CategoryStream crit();

    /**
     * Log a message with alert priority.
     * @param stringFormat Format specifier for the string to write
     * in the log file.
     * @param ... The arguments for stringFormat
     **/
    void alert(const char* stringFormat, ...) ;
    void alert(const std::string& message) ;
    CategoryStream alert();

    /**
     * Log a message with emerg priority.
     * @param stringFormat Format specifier for the string to write
     * in the log file.
     * @param ... The arguments for stringFormat
     **/
    void emerg(const char* stringFormat, ...) ;
    void emerg(const std::string& message) ;
    CategoryStream emerg();

    /**
     * Log a message with notset priority.
     * NB. priority 'notset' is equivalent to 'emerg'.
     * @since 0.2.7
     * @param stringFormat Format specifier for the string to write
     * in the log file.
     * @param ... The arguments for stringFormat
     **/
    void notset(const char* stringFormat, ...);
    void notset(const std::string& message);
    CategoryStream notset();

    /**
         * Set the priority of this Category.
         * @param priority The priority to set. Use Priority::NOTSET to let
         * the category use its parents priority as effective priority.
         * @exception std::invalid_argument if the caller tries to set
         * Priority::NOTSET on the Root Category.
         **/
    virtual void setPriority(PriorityLevel priority);

    /**
     * Returns the assigned Priority, if any, for this Category.
     * @return Priority - the assigned Priority, can be Priority::NOTSET
     **/
    virtual PriorityLevel getPriority() const;
private:
    /* prevent copying and assignment */
    Category(PriorityLevel priority = LOG_NOTSET);
    Category(const Category& other);
    Category& operator=(const Category& other);
    CategoryStream getStream(PriorityLevel priority);
    bool isPriorityEnabled(PriorityLevel priority) const;
    void _logUnconditionally(PriorityLevel priority, const char* format, va_list arguments);
    void _logUnconditionally2(PriorityLevel priority, const std::string& message);
    std::string vform(const char* format, va_list args);
private:
    static Category _category;
    volatile PriorityLevel _priority;
    NetServer* _server;
};

namespace cpplog {
extern Category& root;
}

#endif // LOGCAT_H_20160901
