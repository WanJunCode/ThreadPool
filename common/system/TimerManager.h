
#ifndef __CF_TIMRE_MANAGER_H
#define __CF_TIMRE_MANAGER_H

#include <vector>
#include <stack>
#include<atomic>
#include <mutex>
#include <condition_variable>
#include "Timer.h"

#define TIMER_STACK_SIZE 128

class TimerManager : public Runnable {
public:
    static TimerManager& getInstance() {
        static TimerManager instance;
        return instance;
    }
    ~TimerManager();
public:
    inline void timerMaxSize(int limit = TIMER_STACK_SIZE) {
        timerStackLimit_ = limit;
    };
    Timer *grabTimer();
//Override
public:
    virtual void run();
public:
    void addTimer(Timer *timer);
    void removeTimer(Timer *timer);
protected:
    TimerManager();
    static void timeoutCallback(int fd, short event, void *arg);
private:
    std::mutex mutex_;
    std::atomic<bool> bexit_;
    std::condition_variable expired_cond_;
    std::vector<Timer *> vtimer_;
    std::stack<Timer *> timerStack_;
    size_t timerStackLimit_;
};

#endif
