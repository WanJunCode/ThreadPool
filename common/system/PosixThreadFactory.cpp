#include "PosixThreadFactory.h"
#include <assert.h>
#include <pthread.h>
#include <iostream>
#include "../logcpp/log.h"
#include "Monitor.h"

class PthreadThread : public Thread {
public:
    enum STATE { uninitialized, starting, started, stopping, stopped };

    static const int MB = 1024 * 1024;

    static void* threadMain(void* arg);

private:
    pthread_t pthread_;
    Monitor monitor_;   // guard to protect state_ and also notification
    STATE state_;         // to protect proper thread start behavior
    int policy_;
    int priority_;
    int stackSize_;
    PthreadThread *self_;
    bool detached_;

public:
    PthreadThread(int policy, int priority, int stackSize, bool detached, Runnable *runnable)
        : pthread_(0)
        , state_(uninitialized)
        , policy_(policy)
        , priority_(priority)
        , stackSize_(stackSize)
        , detached_(detached) {
        this->Thread::runnable(runnable);
    }

    ~PthreadThread() {
        if (!detached_) {
            try {
                join();
            } catch (...) {
                // We're really hosed.
            }
        }
    }

    STATE getState() const {
        Synchronized sync(monitor_);
        return state_;
    }

    void setState(STATE newState) {
        Synchronized sync(monitor_);
        state_ = newState;

        // unblock start() with the knowledge that the thread has actually
        // started running, which avoids a race in detached threads.
        if (newState == started) {
            monitor_.notify();
        }
    }

    virtual void start() {
        if (getState() != uninitialized) {
            return;
        }

        pthread_attr_t thread_attr;
        if (pthread_attr_init(&thread_attr) != 0) {
            LOG_CXX(LOG_ERROR) << "pthread_attr_init failed";
            return;
        }

        if (0 != pthread_attr_setdetachstate(&thread_attr,
                                             detached_ ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE)) {
            LOG_CXX(LOG_ERROR) << "pthread_attr_setdetachstate failed";
            return;
        }

        // Set thread stack size
        if (pthread_attr_setstacksize(&thread_attr, MB * stackSize_) != 0) {
            LOG_CXX(LOG_ERROR) << "pthread_attr_setstacksize failed";
            return;
        }

#if _POSIX_THREAD_PRIORITY_SCHEDULING > 0
        if (pthread_attr_setschedpolicy(&thread_attr, policy_) != 0) {
            LOG_CXX(LOG_ERROR) << "pthread_attr_setschedpolicy failed";
            return;
        }
#endif

        //struct sched_param sched_param;
        //sched_param.sched_priority = priority_;

        //// Set thread priority
        //if (pthread_attr_setschedparam(&thread_attr, &sched_param) != 0) {
        //    LOG_CXX(LOG_ERROR) << "pthread_attr_setschedparam failed";
        //    return;
        //}

        // Create reference
        setState(starting);

        Synchronized sync(monitor_);

        if (pthread_create(&pthread_, &thread_attr, threadMain, this) != 0) {
            LOG_CXX(LOG_ERROR) << "pthread_create failed";
            return;
        }

        monitor_.wait();
    }

    virtual void join() {
        if (!detached_ && getState() != uninitialized) {
            void* ignore = NULL;
            int res = pthread_join(pthread_, &ignore);
            detached_ = (res == 0);
            if (res != 0) {
                LOG_C(LOG_ERROR, "PthreadThread::join(): fail with code %d", res);
            }
        }
    }

    virtual Thread::id_t getId() {
        return (Thread::id_t)pthread_;
    }

    virtual Runnable *runnable() const {
        return Thread::runnable();
    }

    virtual void runnable(Runnable *value) {
        Thread::runnable(value);
    }

    void weakRef(PthreadThread *self) {
        assert(self == this);
        self_ = self;
    }
};

void* PthreadThread::threadMain(void* arg) {
    PthreadThread *thread = reinterpret_cast<PthreadThread*>(arg);

    thread->setState(started);

    thread->runnable()->run();

    STATE _s = thread->getState();
    if (_s != stopping && _s != stopped) {
        thread->setState(stopping);
    }

    return (void*)0;
}

/**
 * Converts generic posix thread schedule policy enums into pthread
 * API values.
 */
static int toPthreadPolicy(PosixThreadFactory::POLICY policy) {
    switch (policy) {
    case PosixThreadFactory::OTHER:
        return SCHED_OTHER;
    case PosixThreadFactory::FIFO:
        return SCHED_FIFO;
    case PosixThreadFactory::ROUND_ROBIN:
        return SCHED_RR;
    }
    return SCHED_OTHER;
}

/**
 * Converts relative thread priorities to absolute value based on posix
 * thread scheduler policy
 *
 *  The idea is simply to divide up the priority range for the given policy
 * into the correpsonding relative priority level (lowest..highest) and
 * then pro-rate accordingly.
 */
static int toPthreadPriority(PosixThreadFactory::POLICY policy, PosixThreadFactory::PRIORITY priority) {
    int pthread_policy = toPthreadPolicy(policy);
    int min_priority = 0;
    int max_priority = 0;
#ifdef HAVE_SCHED_GET_PRIORITY_MIN
    min_priority = sched_get_priority_min(pthread_policy);
#endif
#ifdef HAVE_SCHED_GET_PRIORITY_MAX
    max_priority = sched_get_priority_max(pthread_policy);
#endif
    int quanta = (PosixThreadFactory::HIGHEST - PosixThreadFactory::LOWEST) + 1;
    float stepsperquanta = (float)(max_priority - min_priority) / quanta;

    if (priority <= PosixThreadFactory::HIGHEST) {
        return (int)(min_priority + stepsperquanta * priority);
    } else {
        // should never get here for priority increments.
        assert(false);
        return (int)(min_priority + stepsperquanta * PosixThreadFactory::NORMAL);
    }
}

PosixThreadFactory::PosixThreadFactory(POLICY policy, PRIORITY priority, int stackSize, bool detached)
    : ThreadFactory(detached)
    , policy_(policy)
    , priority_(priority)
    , stackSize_(stackSize) {
}

PosixThreadFactory::PosixThreadFactory(bool detached)
    : ThreadFactory(detached)
    , policy_(ROUND_ROBIN)
    , priority_(NORMAL)
    , stackSize_(1) {
}

Thread *PosixThreadFactory::newThread(Runnable *runnable) const {
    PthreadThread * result = new PthreadThread(toPthreadPolicy(policy_),
            toPthreadPriority(policy_, priority_),
            stackSize_,
            isDetached(),
            runnable);
    result->weakRef(result);
    runnable->thread(result);
    return result;
}

int PosixThreadFactory::getStackSize() const {
    return stackSize_;
}

void PosixThreadFactory::setStackSize(int value) {
    stackSize_ = value;
}

PosixThreadFactory::PRIORITY PosixThreadFactory::getPriority() const {
    return priority_;
}

void PosixThreadFactory::setPriority(PRIORITY value) {
    priority_ = value;
}

Thread::id_t PosixThreadFactory::getCurrentThreadId() const {
    return (Thread::id_t)pthread_self();
}

