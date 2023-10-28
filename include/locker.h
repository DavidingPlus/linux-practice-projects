/**
 * @file locker.h
 * @brief 线程同步机制类封装
 * @author 刘治学
 */

#ifndef _LOCKER_H_
#define _LOCKER_H_

#include <pthread.h>
#include <semaphore.h>

#include <exception>

/**
 * @brief 互斥锁类
 */
class Locker {
public:
    /**
     * @brief 默认构造函数重写
     */
    Locker();

    /**
     * @brief 默认析构函数重写
     */

    ~Locker();

    /**
     * @brief 给互斥锁加锁
     * @return bool类型，true表示成功
     */
    bool lock();

    /**
     * @brief 给互斥锁解锁
     * @return bool类型，true表示成功
     */
    bool unlock();

    /**
     * @brief 获得本类中的_mutex对象
     * @return pthread_mutex_t互斥锁类行
     */
    pthread_mutex_t& get_mutex();

private:
    /**
     * @brief 互斥锁
     */
    pthread_mutex_t _mutex;
};

/**
 * @brief 条件变量类
 */
class Cond {
public:
    /**
     * @brief 默认构造函数重写
     */
    Cond();

    /**
     * @brief 默认析构函数重写
     */
    ~Cond();

    /**
     * @brief 满足条件，线程阻塞
     * @param mutex，互斥锁
     * @return bool类型，true表示成功
     */
    bool wait(pthread_mutex_t& mutex);

    /**
     * @brief 阻塞的timewait版本
     * @param mutex，互斥锁
     * @param abstime
     * @return bool类型，true表示成功
     */
    bool timewait(pthread_mutex_t& mutex, struct timespec* abstime);

    /**
     * @brief 满足条件，线程解除阻塞，解除一个或者多个
     * @return bool类型，true表示成功
     */
    bool signal();

    /**
     * @brief 满足条件，线程解除阻塞，解除所有的
     * @return bool类型，true表示成功
     */
    bool broadcast();

    pthread_cond_t& get_cond();

private:
    /**
     * @brief 条件变量
     */
    pthread_cond_t _cond;
};

/**
 * @brief 信号量类
 */
class Sem {
public:
    /**
     * @brief 构造函数
     * @param num，信号量的值，不给默认用0
     */
    Sem(int num = 0);

    /**
     * @brief 重写默认析构函数
     */
    ~Sem();

    /**
     * @brief 等待信号量，信号量的值减减
     * @return bool类型，true表示成功
     */
    bool wait();

    /**
     * @brief 增加信号量，信号量的值加加
     * @return bool类型，true表示成功
     */
    bool post();

private:
    /**
     * @brief 信号量
     */
    sem_t _sem;
};

#endif
