#include "Timer.h"
#include "TimerManager.h"

Timer::Timer(TimerManager *tmMgr)
    : tmMgr_(tmMgr)
    , task_(nullptr)
    , owned_(true) {
}

Timer::~Timer() {
}

void Timer::start(std::function<void()> task, unsigned interval, eTimerType etype) {
    stop();
    task_ = task;
    interval_ = interval;
    type_ = etype;
    tmMgr_->addTimer(this);
}

void Timer::stop() {
    tmMgr_->removeTimer(this);
}

void Timer::run() {
    if (task_) {
        task_();
    }
};
void Timer::reset() {
    interval_ = 0;
    task_ = nullptr;
};
