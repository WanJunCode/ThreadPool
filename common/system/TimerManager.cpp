#include "TimerManager.h"
#include <algorithm>

#define evtimer2_set(ev, cb, arg) event_set(ev, -1, EV_PERSIST, cb, arg)

TimerManager::TimerManager()
    : bexit_(false)
    , timerStackLimit_(TIMER_STACK_SIZE) {
    event_init();
    for (int i = 0; i < 5; ++i) {
        timerStack_.push(new Timer(this));
    }
}


TimerManager::~TimerManager() {
    bexit_ = true;
    std::lock_guard<std::mutex> locker(mutex_);
    while (vtimer_.size()) {
        Timer * ref = vtimer_.back();
        if (!ref->owned_) {
            delete ref;
            vtimer_.pop_back();
        }
    }
    expired_cond_.notify_one();
}

void TimerManager::timeoutCallback(int fd, short event, void *args) {
    Timer *tmProc = (Timer *)args;
    if (tmProc) {
        tmProc->run();
        if (Timer::TIMER_ONCE == tmProc->type_) {
            tmProc->stop();
        }
    }
}

Timer *TimerManager::grabTimer() {
    Timer *timer = nullptr;
    if (timerStack_.empty()) {
        timer = new Timer(this);
        timer->owned_ = false;
    } else {
        std::unique_lock<std::mutex> locker(mutex_);
        if (nullptr != (timer = timerStack_.top())) {
            timerStack_.pop();
            timer->owned_ = false;
        }
    }
    return timer;
}

void TimerManager::addTimer(Timer *timer) {
    {
        /* Initalize one event */
        if (Timer::TIMER_ONCE == timer->type_) {
            evtimer_set(&(timer->tm_), TimerManager::timeoutCallback, timer);
        } else {
            evtimer2_set(&(timer->tm_), TimerManager::timeoutCallback, timer);
        }

        struct timeval tv;
        evutil_timerclear(&tv);
        tv.tv_sec = (int)(timer->interval_);
        tv.tv_usec = (timer->interval_ - tv.tv_sec) * 1000000;

        evtimer_add(&(timer->tm_), &tv);
    }
    std::lock_guard<std::mutex> locker(mutex_);
    vtimer_.push_back(timer);
    expired_cond_.notify_one();
}

void TimerManager::removeTimer(Timer *timer) {
    std::unique_lock<std::mutex> locker(mutex_);
    std::vector<Timer *>::iterator iter = std::find(vtimer_.begin(), vtimer_.end(), timer);
    if (iter != vtimer_.end()) {
        event_del(&(timer->tm_));
        vtimer_.erase(iter);
        if (!timer->owned_) {
            if (timerStack_.size() < timerStackLimit_) {
                timer->reset();
                timerStack_.push(timer);
            } else {
                delete timer;
            }
        }
    }
}

void TimerManager::run() {
    do {
        if (vtimer_.size()) {
            event_dispatch();
        }
        std::unique_lock<std::mutex> locker(mutex_);
        expired_cond_.wait(locker);
    } while (!bexit_);
}