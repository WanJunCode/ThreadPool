#include "Monitor.h"
#include <assert.h>
#include "config.h"
#include "../logcpp/log.h"

class Monitor::Impl {
public:
    Impl()
        : ownedMutex_(new Mutex())
        , mutex_(NULL)
        , condInitialized_(false) {
        init(ownedMutex_);
    }

    Impl(Mutex* mutex)
        : mutex_(NULL)
        , condInitialized_(false) {
        init(mutex);
    }

    Impl(Monitor* monitor)
        : mutex_(NULL)
        , condInitialized_(false) {
        init(&(monitor->mutex()));
    }

    ~Impl() {
        cleanup();
    }

    Mutex& mutex() {
        return *mutex_;
    }
    void lock() {
        mutex().lock();
    }
    void unlock() {
        mutex().unlock();
    }

    /**
     * Exception-throwing version of waitForTimeRelative(), called simply
     * wait(int64) for historical reasons.  Timeout is in milliseconds.
     *
     * If the condition occurs,  this function returns cleanly; on timeout or
     * error an exception is thrown.
     */
    void wait(int64_t timeout_ms) const {
        int result = waitForTimeRelative(timeout_ms);
        if (result != 0) {
            LOG_CXX(LOG_ERROR) << "pthread_cond_wait() or pthread_cond_timedwait() failed";
        }
    }

    /**
     * Waits until the specified timeout in milliseconds for the condition to
     * occur, or waits forever if timeout_ms == 0.
     **/
    int waitForTimeRelative(int64_t timeout_ms) const {
        if (timeout_ms == 0LL) {
            return waitForever();
        }

        struct timespec abstime;
        Util::toTimespec(abstime, Util::currentTime() + timeout_ms);
        return waitForTime(&abstime);
    }

    /**
     * Waits until the absolute time specified using struct THRIFT_TIMESPEC.
     * Returns 0 if condition occurs, THRIFT_ETIMEDOUT on timeout, or an error code.
     */
    int waitForTime(const timespec* abstime) const {
        pthread_mutex_t* mutexImpl = static_cast<pthread_mutex_t*>(mutex_->getUnderlyingImpl());
        assert(mutexImpl);

        // XXX Need to assert that caller owns mutex
        return pthread_cond_timedwait(&pthread_cond_, mutexImpl, abstime);
    }

    int waitForTime(const struct timeval* abstime) const {
        struct timespec temp;
        temp.tv_sec = abstime->tv_sec;
        temp.tv_nsec = abstime->tv_usec * 1000;
        return waitForTime(&temp);
    }
    /**
     * Waits forever until the condition occurs.
     * Returns 0 if condition occurs, or an error code otherwise.
     */
    int waitForever() const {
        pthread_mutex_t* mutexImpl = static_cast<pthread_mutex_t*>(mutex_->getUnderlyingImpl());
        assert(mutexImpl);
        return pthread_cond_wait(&pthread_cond_, mutexImpl);
    }

    void notify() {
        pthread_cond_signal(&pthread_cond_);

    }

    void notifyAll() {
        pthread_cond_broadcast(&pthread_cond_);

    }

private:
    void init(Mutex* mutex) {
        mutex_ = mutex;

        if (0 == pthread_cond_init(&pthread_cond_, NULL)) {
            condInitialized_ = true;
        }

        if (!condInitialized_) {
            cleanup();
            LOG_CXX(LOG_ERROR) << "System Resource Exception";
        }
    }

    void cleanup() {
        if (condInitialized_) {
            condInitialized_ = false;
            CHECK_RETURN_VALUE(pthread_cond_destroy(&pthread_cond_));
        }
    }

    Mutex* ownedMutex_;
    Mutex* mutex_;

    mutable pthread_cond_t pthread_cond_;
    mutable bool condInitialized_;
};

Monitor::Monitor()
    : impl_(new Monitor::Impl()) {
}
Monitor::Monitor(Mutex* mutex)
    : impl_(new Monitor::Impl(mutex)) {
}
Monitor::Monitor(Monitor* monitor)
    : impl_(new Monitor::Impl(monitor)) {
}

Monitor::~Monitor() {
    delete impl_;
}

Mutex& Monitor::mutex() const {
    return impl_->mutex();
}

void Monitor::lock() const {
    impl_->lock();
}

void Monitor::unlock() const {
    impl_->unlock();
}

void Monitor::wait(int64_t timeout) const {
    impl_->wait(timeout);
}

int Monitor::waitForTime(const timeval* abstime) const {
    return impl_->waitForTime(abstime);
}

int Monitor::waitForTimeRelative(int64_t timeout_ms) const {
    return impl_->waitForTimeRelative(timeout_ms);
}

int Monitor::waitForever() const {
    return impl_->waitForever();
}

void Monitor::notify() const {
    impl_->notify();
}

void Monitor::notifyAll() const {
    impl_->notifyAll();
}
