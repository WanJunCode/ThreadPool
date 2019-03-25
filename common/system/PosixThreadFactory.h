#ifndef __CF_POSIX_THREAD_FACTORY_H
#define __CF_POSIX_THREAD_FACTORY_H

#include "Thread.h"
class PosixThreadFactory : public ThreadFactory {

public:
    /**
     * POSIX Thread scheduler policies
     */
    enum POLICY { OTHER, FIFO, ROUND_ROBIN };

    /**
     * POSIX Thread scheduler relative priorities,
     *
     * Absolute priority is determined by scheduler policy and OS. This
     * enumeration specifies relative priorities such that one can specify a
     * priority within a giving scheduler policy without knowing the absolute
     * value of the priority.
     */
    enum PRIORITY {
        LOWEST = 0,
        LOWER = 1,
        LOW = 2,
        NORMAL = 3,
        HIGH = 4,
        HIGHER = 5,
        HIGHEST = 6,
        INCREMENT = 7,
        DECREMENT = 8
    };

    /**
     * Posix thread (pthread) factory.  All threads created by a factory are reference-counted
     * via stdcxx::shared_ptr.  The factory guarantees that threads and the Runnable tasks
     * they host will be properly cleaned up once the last strong reference to both is
     * given up.
     *
     * Threads are created with the specified policy, priority, stack-size and detachable-mode
     * detached means the thread is free-running and will release all system resources the
     * when it completes.  A detachable thread is not joinable.  The join method
     * of a detachable thread will return immediately with no error.
     *
     * By default threads are not joinable.
     */
    PosixThreadFactory(POLICY policy = ROUND_ROBIN,
                       PRIORITY priority = NORMAL,
                       int stackSize = 1,
                       bool detached = false);

    /**
     * Provide a constructor compatible with the other factories
     * The default policy is POLICY::ROUND_ROBIN.
     * The default priority is PRIORITY::NORMAL.
     * The default stackSize is 1.
     */
    PosixThreadFactory(bool detached);

    // From ThreadFactory;
    Thread *newThread(Runnable *runnable) const;

    // From ThreadFactory;
    Thread::id_t getCurrentThreadId() const;

    /**
     * Gets stack size for newly created threads
     *
     * @return int size in megabytes
     */
    virtual int getStackSize() const;

    /**
     * Sets stack size for newly created threads
     *
     * @param value size in megabytes
     */
    virtual void setStackSize(int value);

    /**
     * Gets priority relative to current policy
     */
    virtual PRIORITY getPriority() const;

    /**
     * Sets priority relative to current policy
     */
    virtual void setPriority(PRIORITY priority);

private:
    POLICY policy_;
    PRIORITY priority_;
    int stackSize_;
};

#endif
