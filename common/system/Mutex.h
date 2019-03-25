#ifndef __CF_MUTEX_H
#define __CF_MUTEX_H

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include "config.h"
#include "../utils/utime.h"

class Mutex {
public:
    Mutex() {
        CHECK_RETURN_VALUE(pthread_mutex_init(&pthread_mutex_, NULL));
    };
    ~Mutex() {
        CHECK_RETURN_VALUE(pthread_mutex_destroy(&pthread_mutex_));
    }

    void lock() {
        CHECK_RETURN_VALUE(pthread_mutex_lock(&pthread_mutex_));
    };
    bool trylock() {
        return pthread_mutex_trylock(&pthread_mutex_);
    }
    bool timedlock(int64_t milliseconds) {
        struct timespec ts;
        Util::toTimespec(ts, milliseconds + Util::currentTime());
        int ret = pthread_mutex_timedlock(&pthread_mutex_, &ts);
        if (ret == 0) {
            return true;
        } else if (ret == ETIMEDOUT) {
            return false;
        }
        fprintf(stderr, "%d = pthread_mutex_timedlock(&pthread_mutex_, &ts)", ret);
        return false;
    }
    void unlock() {
        CHECK_RETURN_VALUE(pthread_mutex_unlock(&pthread_mutex_));
    }

    pthread_mutex_t* getUnderlyingImpl() {
        return &pthread_mutex_;
    }
private:
    pthread_mutex_t pthread_mutex_;
};

class Guard {
public:
    Guard(Mutex& value, int64_t timeout = 0)
        : mutex_(&value) {
        if (timeout == 0) {
            value.lock();
        } else if (timeout < 0) {
            if (!value.trylock()) {
                mutex_ = NULL;
            }
        } else {
            if (!value.timedlock(timeout)) {
                mutex_ = NULL;
            }
        }
    }

    ~Guard() {
        if (mutex_) {
            mutex_->unlock();
        }
    }

    operator bool() const {
        return (mutex_ != NULL);
    }
private:
    Mutex *mutex_;
};

class ReadWriteMutex {
public:
    ReadWriteMutex() {
        CHECK_RETURN_VALUE(pthread_rwlock_init(&lock_, NULL));
    };
    virtual ~ReadWriteMutex() {
        CHECK_RETURN_VALUE(pthread_rwlock_destroy(&lock_));
    }

public:
    // these get the lock and block until it is done successfully
    void acquireRead() {
        CHECK_RETURN_VALUE(pthread_rwlock_rdlock(&lock_));
    }
    void acquireWrite() {
        CHECK_RETURN_VALUE(pthread_rwlock_wrlock(&lock_));
    }

    // these attempt to get the lock, returning false immediately if they fail
    bool attemptRead() {
        CHECK_RETURN_VALUE(pthread_rwlock_tryrdlock(&lock_));
    }
    bool attemptWrite() {
        CHECK_RETURN_VALUE(pthread_rwlock_trywrlock(&lock_));
    }

    // this releases both read and write locks
    void release() {
        CHECK_RETURN_VALUE(pthread_rwlock_unlock(&lock_));
    }

//Attibute
private:
    pthread_rwlock_t lock_;
};


// Can be used as second argument to RWGuard to make code more readable
// as to whether we're doing acquireRead() or acquireWrite().
enum RWGuardType { RW_READ = 0, RW_WRITE = 1 };

class RWGuard {
public:
    RWGuard(ReadWriteMutex& value, RWGuardType type = RW_READ)
        : rw_mutex_(value) {
        if (type == RW_WRITE) {
            rw_mutex_.acquireWrite();
        } else {
            rw_mutex_.acquireRead();
        }
    }
    ~RWGuard() {
        rw_mutex_.release();
    }

private:
    ReadWriteMutex& rw_mutex_;
};

#endif
