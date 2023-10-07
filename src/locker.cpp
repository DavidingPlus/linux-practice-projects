/**
 * @file locker.cpp
 * @brief 线程同步机制类封装
 * @author 刘治学
 */

#include "locker.h"

//-----------------------------------------------------------
Locker::Locker() {
    if (0 != pthread_mutex_init(&_mutex, nullptr))
        throw std::exception();  // 抛出一个异常
}

Locker::~Locker() {
    pthread_mutex_destroy(&_mutex);
}

bool Locker::lock() {
    return 0 == pthread_mutex_lock(&_mutex);
}

bool Locker::unlock() {
    return 0 == pthread_mutex_unlock(&_mutex);
}

pthread_mutex_t& Locker::get_mutex() {
    return _mutex;
}

//-----------------------------------------------------------
Cond::Cond() {
    if (0 != pthread_cond_init(&_cond, nullptr))
        throw std::exception();
}

Cond::~Cond() {
    pthread_cond_destroy(&_cond);
}

bool Cond::wait(pthread_mutex_t& mutex) {
    return 0 == pthread_cond_wait(&_cond, &mutex);
}

bool Cond::timewait(pthread_mutex_t& mutex, timespec* abstime) {
    return 0 == pthread_cond_timedwait(&_cond, &mutex, abstime);
}

bool Cond::signal() {
    return 0 == pthread_cond_signal(&_cond);
}

bool Cond::broadcast() {
    return 0 == pthread_cond_broadcast(&_cond);
}

pthread_cond_t& Cond::get_cond() {
    return _cond;
}

//-----------------------------------------------------------
Sem::Sem(int num) {
    if (0 != sem_init(&_sem, 0, num))
        throw std::exception();
}

Sem::~Sem() {
    sem_destroy(&_sem);
}

bool Sem::wait() {
    return 0 == sem_wait(&_sem);
}

bool Sem::post() {
    return 0 == sem_post(&_sem);
}
