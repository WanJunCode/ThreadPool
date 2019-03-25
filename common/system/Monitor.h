#ifndef __CF_MONITOR_H
#define __CF_MONITOR_H

#include <sys/time.h>
#include "config.h"
#include "Mutex.h"

/**
 * A monitor is a combination mutex and condition-event.  Waiting and
 * notifying condition events requires that the caller own the mutex.  Mutex
 * lock and unlock operations can be performed independently of condition
 * events.  This is more or less analogous to java.lang.Object multi-thread
 * operations.
 */
class Monitor {
public:
    /** Creates a new mutex, and takes ownership of it. */
    Monitor();

    /** Uses the provided mutex without taking ownership. */
    explicit Monitor(Mutex* mutex);

    /** Uses the mutex inside the provided Monitor without taking ownership. */
    explicit Monitor(Monitor* monitor);

    /** Deallocates the mutex only if we own it. */
    virtual ~Monitor();

public:
    Mutex& mutex() const;

    virtual void lock() const;

    virtual void unlock() const;

    /**
     * Waits a maximum of the specified timeout in milliseconds for the condition
     * to occur, or waits forever if timeout_ms == 0.
     *
     * Returns 0 if condition occurs, THRIFT_ETIMEDOUT on timeout, or an error code.
     */
    int waitForTimeRelative(int64_t timeout_ms) const;

    /**
     * Waits until the absolute time specified using struct timeval.
     * Returns 0 if condition occurs, THRIFT_ETIMEDOUT on timeout, or an error code.
     */
    int waitForTime(const struct timeval* abstime) const;

    /**
     * Waits forever until the condition occurs.
     * Returns 0 if condition occurs, or an error code otherwise.
     */
    int waitForever() const;

    /**
     * Exception-throwing version of waitForTimeRelative(), called simply
     * wait(int64) for historical reasons.  Timeout is in milliseconds.
     *
     * If the condition occurs,  this function returns cleanly; on timeout or
     * error an exception is thrown.
     */
    void wait(int64_t timeout_ms = 0LL) const;

    /** Wakes up one thread waiting on this monitor. */
    virtual void notify() const;

    /** Wakes up all waiting threads on this monitor. */
    virtual void notifyAll() const;

private:
    class Impl;

    Impl* impl_;
};

class Synchronized {
public:
    Synchronized(const Monitor* monitor) : g(monitor->mutex()) {}
    Synchronized(const Monitor& monitor) : g(monitor.mutex()) {}

private:
    Guard g;
};

#endif
