#include "ThreadManager.h"
#include <stdexcept>
#include <deque>
#include <set>
#include <map>
#include "../logcpp/log.h"
#include "../utils/utime.h"
#include "Monitor.h"
#include "Thread.h"

class ThreadManager::Impl : public ThreadManager {
    friend class ThreadManager::Task;
    friend class ThreadManager::Worker;
public:
    Impl()
        : workerCount_(0)
        , workerMaxCount_(0)
        , idleCount_(0)
        , pendingTaskCountMax_(0)
        , expiredCount_(0)
        , state_(ThreadManager::UNINITIALIZED)
        , threadFactory_(NULL)
        , monitor_(&mutex_)
        , maxMonitor_(&mutex_)
        , workerMonitor_(&mutex_) {
    }

    ~Impl() {
        stop();
    }

public:
    virtual void start();
    virtual void stop();

    virtual ThreadManager::STATE state() const {
        return state_;
    }

    virtual std::shared_ptr<ThreadFactory> threadFactory() {
        Guard g(mutex_);
        return threadFactory_;
    }

    virtual void threadFactory(std::shared_ptr<ThreadFactory> value) {
        Guard g(mutex_);
        if (threadFactory_ && threadFactory_->isDetached() != value->isDetached()) {
            //throw InvalidArgumentException();
            LOG_CXX(LOG_ERROR) << "thread is invalid argument";
            return;
        }
        threadFactory_ = value;
    }

    virtual void addWorker(size_t value);

    virtual void removeWorker(size_t value);

    virtual size_t idleWorkerCount() const {
        return idleCount_;
    }

    virtual size_t workerCount() {
        Guard g(mutex_);
        return workerCount_;
    }

    virtual size_t pendingTaskCount() {
        Guard g(mutex_);
        return tasks_.size();
    }

    virtual size_t totalTaskCount() {
        Guard g(mutex_);
        return tasks_.size() + workerCount_ - idleCount_;
    }

    virtual size_t pendingTaskCountMax() {
        Guard g(mutex_);
        return pendingTaskCountMax_;
    }

    virtual size_t expiredTaskCount() {
        Guard g(mutex_);
        return expiredCount_;
    }

    virtual void pendingTaskCountMax(const size_t value) {
        Guard g(mutex_);
        pendingTaskCountMax_ = value;
    }

    virtual void add(std::shared_ptr<Runnable> value, int64_t timeout, int64_t expiration);

    virtual void remove(std::shared_ptr<Runnable> task);

    virtual std::shared_ptr<Runnable> removeNextPending();

    virtual void removeExpiredTasks() {
        removeExpired(false);
    }

    virtual void setExpireCallback(ExpireCallback expireCallback);

private:
    /**
     * Remove one or more expired tasks.
     * \param[in]  justOne  if true, try to remove just one task and return
     */
    void removeExpired(bool justOne);

    /**
     * \returns whether it is acceptable to block, depending on the current thread id
     */
    bool canSleep() const;

    /**
     * Lowers the maximum worker count and blocks until enough worker threads complete
     * to get to the new maximum worker limit.  The caller is responsible for acquiring
     * a lock on the class mutex_.
     */
    void removeWorkersUnderLock(size_t value);

private:
    size_t workerCount_;
    size_t workerMaxCount_;
    size_t idleCount_;
    size_t pendingTaskCountMax_;
    size_t expiredCount_;
    ExpireCallback expireCallback_;

    ThreadManager::STATE state_;
    std::shared_ptr<ThreadFactory> threadFactory_;

    typedef std::deque< std::shared_ptr<Task> > TaskQueue;
    TaskQueue tasks_;
    Mutex mutex_;
    Monitor monitor_;
    Monitor maxMonitor_;
    Monitor workerMonitor_;       // used to synchronize changes in worker count

    std::set<Thread *> workers_;
    std::set<Thread *> deadWorkers_;
    std::map<const Thread::id_t, Thread*> idMap_;
};

class ThreadManager::Task : public Runnable {
public:
    enum STATE { WAITING, EXECUTING, TIMEDOUT, COMPLETE };

    Task(std::shared_ptr<Runnable> runnable, int64_t expiration = 0LL)
        : runnable_(runnable),
          state_(WAITING),
          expireTime_(expiration != 0LL ? Util::currentTime() + expiration : 0LL) {}

    ~Task() {}

public:
    void run() {
        if (state_ == EXECUTING) {
            runnable_->run();
            state_ = COMPLETE;
        }
    }

    inline std::shared_ptr<Runnable> getRunnable() {
        return runnable_;
    }

    inline int64_t getExpireTime() const {
        return expireTime_;
    }

private:
    std::shared_ptr<Runnable> runnable_;
    friend class ThreadManager::Worker;
    STATE state_;
    int64_t expireTime_;
};

class ThreadManager::Worker : public Runnable {
    friend class ThreadManager::Impl;
    enum STATE { UNINITIALIZED, STARTING, STARTED, STOPPING, STOPPED };
public:
    Worker(ThreadManager::Impl* manager)
        : manager_(manager), state_(UNINITIALIZED) {}
    ~Worker() {}

private:
    bool isActive() const {
        return (manager_->workerCount_ <= manager_->workerMaxCount_)
               || (manager_->state_ == JOINING && !manager_->tasks_.empty());
    }

public:
    void run() {
        Guard g(manager_->mutex_);
        bool active = manager_->workerCount_ < manager_->workerMaxCount_;
        if (active) {
            if (++manager_->workerCount_ == manager_->workerMaxCount_) {
                manager_->workerMonitor_.notify();
            }
        }

        while (active) {
            active = isActive();

            while (active && manager_->tasks_.empty()) {
                manager_->idleCount_++;
                manager_->monitor_.wait();
                active = isActive();
                manager_->idleCount_--;
            }
            std::shared_ptr<ThreadManager::Task> task;
            if (active) {
                if (!manager_->tasks_.empty()) {
                    task = manager_->tasks_.front();
                    manager_->tasks_.pop_front();
                    if (task->state_ == ThreadManager::Task::WAITING) {
                        // If the state is changed to anything other than EXECUTING or TIMEDOUT here
                        // then the execution loop needs to be changed below.
                        task->state_ =
                            (task->getExpireTime() && task->getExpireTime() < Util::currentTime()) ?
                            ThreadManager::Task::TIMEDOUT :
                            ThreadManager::Task::EXECUTING;
                    }
                }

                /* If we have a pending task max and we just dropped below it, wakeup any
                    thread that might be blocked on add. */
                if (manager_->pendingTaskCountMax_ != 0
                        && manager_->tasks_.size() <= manager_->pendingTaskCountMax_ - 1) {
                    manager_->maxMonitor_.notify();
                }
            }

            /**
             * Execution - not holding a lock
             */
            if (task) {
                if (task->state_ == ThreadManager::Task::EXECUTING) {

                    // Release the lock so we can run the task without blocking the thread manager
                    manager_->mutex_.unlock();

                    try {
                        task->run();
                    } catch (const std::exception& e) {
                        //GlobalOutput.printf("[ERROR] task->run() raised an exception: %s", e.what());
                        LOG_CXX(LOG_ERROR) << "task->run() raised an exception:" << e.what();
                    } catch (...) {
                        //GlobalOutput.printf("[ERROR] task->run() raised an unknown exception");
                        LOG_CXX(LOG_ERROR) << "task->run() raised an unknown exception";
                    }

                    // Re-acquire the lock to proceed in the thread manager
                    manager_->mutex_.lock();

                } else if (manager_->expireCallback_) {
                    // The only other state the task could have been in is TIMEDOUT (see above)
                    manager_->expireCallback_(task->getRunnable());
                    manager_->expiredCount_++;
                }
            }
        }

        /**
         * Final accounting for the worker thread that is done working
         */
        manager_->deadWorkers_.insert(this->thread());
        if (--manager_->workerCount_ == manager_->workerMaxCount_) {
            manager_->workerMonitor_.notify();
        }
    }

private:
    ThreadManager::Impl* manager_;
    STATE state_;
};

void ThreadManager::Impl::addWorker(size_t value) {
    std::set<Thread*> newThreads;
    for (size_t i = 0; i < value; ++i) {
        ThreadManager::Worker* worker = new ThreadManager::Worker(this);
        newThreads.insert(threadFactory_->newThread(worker));
    }

    Guard g(mutex_);
    workerMaxCount_ += value;
    workers_.insert(newThreads.begin(), newThreads.end());

    for (std::set<Thread*>::iterator iter = newThreads.begin(); iter != newThreads.end(); ++iter) {
        ThreadManager::Worker *worker = dynamic_cast<ThreadManager::Worker *>((*iter)->runnable());
        worker->state_ = ThreadManager::Worker::STARTING;
        (*iter)->start();
        idMap_.insert(std::pair<const Thread::id_t, Thread*>((*iter)->getId(), *iter));
    }

    while (workerCount_ != workerMaxCount_) {
        workerMonitor_.wait();
    }
}

void ThreadManager::Impl::start() {
    Guard g(mutex_);
    if (state_ == ThreadManager::STOPPED) {
        return;
    }

    if (state_ == ThreadManager::UNINITIALIZED) {
        if (!threadFactory_) {
            //throw InvalidArgumentException();
            LOG_CXX(LOG_ERROR) << "threadFactory is nullptr";
            return;
        }
        state_ = ThreadManager::STARTED;
        monitor_.notifyAll();
    }

    while (state_ == STARTING) {
        monitor_.wait();
    }
}

void ThreadManager::Impl::stop() {
    Guard g(mutex_);
    bool doStop = false;

    if (state_ != ThreadManager::STOPPING &&
            state_ != ThreadManager::JOINING &&
            state_ != ThreadManager::STOPPED) {
        doStop = true;
        state_ = ThreadManager::JOINING;
    }

    if (doStop) {
        removeWorkersUnderLock(workerCount_);
    }

    state_ = ThreadManager::STOPPED;
}

void ThreadManager::Impl::removeWorker(size_t value) {
    Guard g(mutex_);
    removeWorkersUnderLock(value);
}

void ThreadManager::Impl::removeWorkersUnderLock(size_t value) {
    if (value > workerMaxCount_) {
        //throw InvalidArgumentException();
        LOG_CXX(LOG_ERROR) << "Invalid Argument";
        return ;
    }

    workerMaxCount_ -= value;

    if (idleCount_ > value) {
        // There are more idle workers than we need to remove,
        // so notify enough of them so they can terminate.
        for (size_t ix = 0; ix < value; ix++) {
            monitor_.notify();
        }
    } else {
        // There are as many or less idle workers than we need to remove,
        // so just notify them all so they can terminate.
        monitor_.notifyAll();
    }

    while (workerCount_ != workerMaxCount_) {
        workerMonitor_.wait();
    }

    for (std::set<Thread *>::iterator ix = deadWorkers_.begin(); ix != deadWorkers_.end(); ++ix) {

        // when used with a joinable thread factory, we join the threads as we remove them
        if (!threadFactory_->isDetached()) {
            (*ix)->join();
        }

        idMap_.erase((*ix)->getId());
        workers_.erase(*ix);
    }

    deadWorkers_.clear();
}

bool ThreadManager::Impl::canSleep() const {
    const Thread::id_t id = threadFactory_->getCurrentThreadId();
    return idMap_.find(id) == idMap_.end();
}

void ThreadManager::Impl::add(std::shared_ptr<Runnable> value, int64_t timeout, int64_t expiration) {
    Guard g(mutex_, timeout);

    if (!g) {
        //throw TimedOutException();
        LOG_C(LOG_ERROR, "add task is timeout:%ld", timeout);
        return;
    }

    if (state_ != ThreadManager::STARTED) {
        LOG_CXX(LOG_ERROR) << "ThreadManager::Impl::add ThreadManager not started";
        return;
    }

    // if we're at a limit, remove an expired task to see if the limit clears
    if (pendingTaskCountMax_ > 0 && (tasks_.size() >= pendingTaskCountMax_)) {
        removeExpired(true);
    }

    if (pendingTaskCountMax_ > 0 && (tasks_.size() >= pendingTaskCountMax_)) {
        if (canSleep() && timeout >= 0) {
            while (pendingTaskCountMax_ > 0 && tasks_.size() >= pendingTaskCountMax_) {
                // This is thread safe because the mutex is shared between monitors.
                maxMonitor_.wait(timeout);
            }
        } else {
            //throw TooManyPendingTasksException();
            LOG_CXX(LOG_ERROR) << "Too Many Pending Tasks Exception";
            return;
        }
    }

    tasks_.push_back(std::shared_ptr<ThreadManager::Task>(new ThreadManager::Task(value, expiration)));

    // If idle thread is available notify it, otherwise all worker threads are
    // running and will get around to this task in time.
    if (idleCount_ > 0) {
        monitor_.notify();
    }
}

void ThreadManager::Impl::remove(std::shared_ptr<Runnable> task) {
    Guard g(mutex_);
    if (state_ != ThreadManager::STARTED) {
        /*throw IllegalStateException(
            "ThreadManager::Impl::remove ThreadManager not "
            "started");*/
        LOG_CXX(LOG_ERROR) << "IllegalStateException ThreadManager::Impl::remove ThreadManager not started";
        return;
    }

    for (TaskQueue::iterator it = tasks_.begin(); it != tasks_.end(); ++it) {
        if ((*it)->getRunnable() == task) {
            tasks_.erase(it);
            return;
        }
    }
}

std::shared_ptr<Runnable> ThreadManager::Impl::removeNextPending() {
    Guard g(mutex_);
    if (state_ != ThreadManager::STARTED) {
        LOG_CXX(LOG_ERROR) << "ThreadManager::Impl::removeNextPending ThreadManager not started";
        return NULL;
    }

    if (tasks_.empty()) {
        return NULL;
    }

    std::shared_ptr<ThreadManager::Task> task = tasks_.front();
    tasks_.pop_front();

    return task->getRunnable();
}

void ThreadManager::Impl::removeExpired(bool justOne) {
    // this is always called under a lock
    int64_t now = 0LL;

    for (TaskQueue::iterator it = tasks_.begin(); it != tasks_.end(); ) {
        if (now == 0LL) {
            now = Util::currentTime();
        }

        if ((*it)->getExpireTime() > 0LL && (*it)->getExpireTime() < now) {
            if (expireCallback_) {
                expireCallback_((*it)->getRunnable());
            }
            it = tasks_.erase(it);
            ++expiredCount_;
            if (justOne) {
                return;
            }
        } else {
            ++it;
        }
    }
}

void ThreadManager::Impl::setExpireCallback(ExpireCallback expireCallback) {
    Guard g(mutex_);
    expireCallback_ = expireCallback;
}

class SimpleThreadManager : public ThreadManager::Impl {
public:
    SimpleThreadManager(size_t workerCount = 4, size_t pendingTaskCountMax = 0)
        : workerCount_(workerCount)
        , pendingTaskCountMax_(pendingTaskCountMax) {}

    void start() {
        ThreadManager::Impl::pendingTaskCountMax(pendingTaskCountMax_);
        ThreadManager::Impl::start();
        addWorker(workerCount_);
    }

private:
    const size_t workerCount_;
    const size_t pendingTaskCountMax_;
};

ThreadManager *ThreadManager::noblockingTaskThreadManager() {
    return new ThreadManager::Impl();
}

ThreadManager *ThreadManager::blockingTaskThreadManager(size_t count, size_t pendingTaskCountMax) {
    return  new SimpleThreadManager(count, pendingTaskCountMax);
}
