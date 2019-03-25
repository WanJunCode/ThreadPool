#ifndef __CF_THREAD_MANAGER_H
#define __CF_THREAD_MANAGER_H

#include <memory>
#include <sys/types.h>

class Runnable;
class ThreadFactory;
class ThreadManager {
public:
    typedef void(*ExpireCallback)(std::shared_ptr<Runnable>);
public:
    virtual ~ThreadManager() {};

public:
    /**
    * Starts the thread manager.
    */
    virtual void start() = 0;

    /**
    * Stops the thread manager.
    */
    virtual void stop() = 0;

    enum STATE { UNINITIALIZED, STARTING, STARTED, JOINING, STOPPING, STOPPED };
    virtual STATE state() const = 0;

    /**
    * returns the current thread factory
    */
    virtual std::shared_ptr<ThreadFactory> threadFactory() = 0;

    /**
    * Set the thread factory.
    */
    virtual void threadFactory(std::shared_ptr<ThreadFactory> value) = 0;

    /**
    * Adds worker thread(s).
    */
    virtual void addWorker(size_t value = 1) = 0;

    /**
    * Removes worker thread(s).
    * Threads are joined if the thread factory detached disposition allows it.
    * Blocks until the number of worker threads reaches the new limit.
    */
    virtual void removeWorker(size_t value = 1) = 0;

    /**
    * Gets the current number of idle worker threads
    */
    virtual size_t idleWorkerCount() const = 0;

    /**
    * Gets the current number of total worker threads
    */
    virtual size_t workerCount() = 0;

    /**
    * Gets the current number of pending tasks
    */
    virtual size_t pendingTaskCount() = 0;

    /**
    * Gets the current number of pending and executing tasks
    */
    virtual size_t totalTaskCount() = 0;

    /**
    * Gets the maximum pending task count.  0 indicates no maximum
    */
    virtual size_t pendingTaskCountMax() = 0;

    /**
    * Gets the number of tasks which have been expired without being run since start() was called.
    */
    virtual size_t expiredTaskCount() = 0;

    /**
    * Adds a task to be executed at some time in the future by a worker thread.
    *
    * This method will block if pendingTaskCountMax() in not zero and pendingTaskCount()
    * is greater than or equalt to pendingTaskCountMax().
    *  If this method is called in the context of a ThreadManager worker thread it will throw a
    */
    virtual void add(std::shared_ptr<Runnable> task, int64_t timeout = 0LL, int64_t expiration = 0LL) = 0;

    /**
    * Removes a pending task
    */
    virtual void remove(std::shared_ptr<Runnable> task) = 0;

    /**
    * Remove the next pending task which would be run.
    */
    virtual std::shared_ptr<Runnable> removeNextPending() = 0;

    /**
    * Remove tasks from front of task queue that have expired.
    */
    virtual void removeExpiredTasks() = 0;

    /**
    * Set a callback to be called when a task is expired and not run.
    */
    virtual void setExpireCallback(ExpireCallback expireCallback) = 0;

public:
    /**
    * Creates a simple thread manager the uses count number of worker threads and has
    * a pendingTaskCountMax maximum pending tasks. The default, 0, specified no limit
    * on pending tasks
    */
    static ThreadManager *noblockingTaskThreadManager();

    static ThreadManager *blockingTaskThreadManager(size_t count = 4, size_t pendingTaskCountMax = 0);

protected:
    ThreadManager() {};

public:
    class Task; 	// work task
    class Worker; 	//thread body
    class Impl;		//thread impl
};

#endif