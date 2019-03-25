/**
* Copyrights(C) 2015 - 2016, ORVIBO
* All rights reserved
*
* File:
Queue_s.h
*
* 线程安全队列封装
*
* 说明 :
*1、Queue_s：
*         基于模板stl中queue实现线程安全
* 2、Priority_Queue_s：
*         基于模板stl中priority_queue实现线程安全
* 3、用法跟普通模板用法一致
*
* Change Logs :
*Date         Author      Notes
* 2016 - 08 - 04   chenfei     创建
* */

#ifndef __CF_QUEUE_S_H
#define __CF_QUEUE_S_H

#include <queue>
#include <list>
#include <functional>   // std::less
#include <algorithm>    // std::sort, std::includes
#include "Monitor.h"
#include "Mutex.h"

template <class T, class Sequence = std::list<T> >
class Queue_s : public std::queue < T, Sequence > {
public:
    explicit Queue_s();
    ~Queue_s();
public:
    void push(const T& val);
    T pop_front(bool &bstat, double seconds = 0);
    size_t size() const;
    void swap(Queue_s& t);
    void wake_all();
protected:
    void pop();
    Monitor* pcond_;
};

template <class T, class Sequence = std::deque<T>, class Compare = std::less<T> >
class Priority_Queue_s : public std::priority_queue < T, Sequence, Compare > {
public:
    explicit Priority_Queue_s();
    ~Priority_Queue_s();
public:
    void push(const T& val);
    T pop_front(bool &bstat, double seconds = 0);
    size_t size() const;
    void swap(Priority_Queue_s& t);
    void wake_all();
protected:
    void pop();
    Monitor* pcond_;
};

template <class T, class Sequence>
Queue_s<T, Sequence>::Queue_s()
    : pcond_(new Monitor()) {
    assert(pcond_);
}

template <class T, class Sequence>
Queue_s<T, Sequence>::~Queue_s() {
    if (pcond_)
        delete pcond_;
    pcond_ = NULL;
}

template <class T, class Sequence>
void Queue_s<T, Sequence>::push(const T& val) {
    Guard ug(pcond_->mutex());
    std::queue<T, Sequence>::push(val);
    pcond_->notify();
}

template <class T, class Sequence>
void Queue_s<T, Sequence>::pop() {
    Guard ug(pcond_->mutex());
    std::queue<T, Sequence>::pop();
}

template <class T, class Sequence>
T Queue_s<T, Sequence>::pop_front(bool &bstat, double seconds) {
    Guard ug(pcond_->mutex());
    if (std::queue<T, Sequence>::empty())
        pcond_->wait(seconds);

    if (!std::queue<T, Sequence>::empty()) {
        T tmp(std::queue<T, Sequence>::front());
        std::queue<T, Sequence>::pop();
        bstat = true;
        return tmp;
    }
    bstat = false;
    return T();
}

template <class T, class Sequence>
size_t Queue_s<T, Sequence>::size() const {
    Guard ug(pcond_->mutex());
    return std::queue<T, Sequence>::size();
}

template <class T, class Sequence>
void Queue_s<T, Sequence>::wake_all() {
    pcond_->notifyAll();
}

template <class T, class Sequence>
void Queue_s<T, Sequence>::swap(Queue_s& t) {
    Guard ug(pcond_->mutex());
    std::queue<T, Sequence>::swap(t);
}


///////////////////////////////////////////////////

template <class T, class Sequence, class Compare>
Priority_Queue_s<T, Sequence, Compare>::Priority_Queue_s()
    : pcond_(new Monitor()) {
    assert(pcond_);
}

template <class T, class Sequence, class Compare>
Priority_Queue_s<T, Sequence, Compare>::~Priority_Queue_s() {
    if (pcond_)
        delete pcond_;
    pcond_ = NULL;
}

template <class T, class Sequence, class Compare>
void Priority_Queue_s<T, Sequence, Compare>::push(const T& val) {
    Guard ug(pcond_->mutex());
    std::priority_queue<T, Sequence, Compare>::push(val);
    pcond_->notify();
}

template <class T, class Sequence, class Compare>
void Priority_Queue_s<T, Sequence, Compare>::pop() {
    Guard ug(pcond_->mutex());
    std::priority_queue<T, Sequence, Compare>::pop();
}

template <class T, class Sequence, class Compare>
T Priority_Queue_s<T, Sequence, Compare>::pop_front(bool &bstat, double seconds) {
    Guard ug(pcond_->mutex());
    if (std::priority_queue<T, Sequence, Compare>::empty())
        pcond_->wait(seconds);

    if (!std::priority_queue<T, Sequence>::empty()) {
        T tmp(std::priority_queue<T, Sequence, Compare>::top());
        std::priority_queue<T, Sequence, Compare>::pop();
        bstat = true;
        return tmp;
    }
    bstat = false;
    return T();
}

template <class T, class Sequence, class Compare>
size_t Priority_Queue_s<T, Sequence, Compare>::size() const {
    Guard ug(pcond_->mutex());
    return std::priority_queue<T, Sequence, Compare>::size();
}

template <class T, class Sequence, class Compare>
void Priority_Queue_s<T, Sequence, Compare>::wake_all() {
    pcond_->notifyAll();
}

template <class T, class Sequence, class Compare>
void Priority_Queue_s<T, Sequence, Compare>::swap(Priority_Queue_s& t) {
    Guard ug(pcond_->mutex());
    std::priority_queue<T, Sequence, Compare>::swap(t);
}

#endif
