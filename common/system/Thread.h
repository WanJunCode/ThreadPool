#ifndef __CF_TTHREAD_H
#define __CF_TTHREAD_H

#include <pthread.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

class Thread;
class Runnable {
public:
    virtual ~Runnable() {};

public:
    virtual void run() = 0;
    /**
    * Gets the thread object that is hosting this runnable object
    */
    virtual Thread* thread() {
        return thread_;
    }
    /**
    * Sets the thread that is executing this object.
    */
    virtual void thread(Thread *value) {
        thread_ = value;
    }

private:
    Thread* thread_;
};

class Thread {
public:
    typedef pthread_t id_t;
public:
    virtual ~Thread() {};
    /**
    * Starts the thread.
    */
    virtual void start() = 0;
    /**
    * Join this thread.
    */
    virtual void join() = 0;
    /**
    * Gets the thread's platform-specific ID
    */
    virtual id_t getId() = 0;
    /**
    * Gets the runnable object this thread is hosting
    */
    virtual Runnable* runnable() const {
        return runnable_;
    }

public:

    static inline bool is_current(id_t t) {
        return pthread_equal(pthread_self(), t);
    }
    static inline id_t get_current() {
        return pthread_self();
    }

protected:
    virtual void runnable(Runnable* value) {
        runnable_ = value;
    }

private:
    Runnable* 	runnable_;
};

class ThreadFactory {
public:
    virtual ~ThreadFactory() { }

public:
    /**
    * Gets current detached mode
    */
    inline bool isDetached() const {
        return detached_;
    }
    /**
    * Sets the detached disposition of newly created threads.
    */
    inline void setDetached(bool detached) {
        detached_ = detached;
    }

public:
    /**
    * Create a new thread.
    */
    virtual Thread* newThread(Runnable* runnable) const = 0;
    /**
    * Gets the current thread id or unknown_thread_id if the current thread is not a thrift thread
    */
    virtual Thread::id_t getCurrentThreadId() const = 0;

public:
    /**
    * For code readability define the unknown/undefined thread id
    */
    static const Thread::id_t unknown_thread_id;

protected:
    ThreadFactory(bool detached)
        : detached_(detached) {}
private:
    bool detached_;
};

#endif