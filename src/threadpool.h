/**
 * @file threadpool.h
 * @brief 线程池封装
 * @author 刘治学
 */

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <pthread.h>

#include <exception>
#include <iostream>
#include <list>

#include "http_conn.h"
#include "locker.h"

/**
 * @brief 线程池类，定义成模板类是为了代码复用
 * @tparam Task，模板参数，就是任务类
 */
template <class Task>
class Thread_Pool {
public:
    /**
     * @brief 构造函数
     * @param thread_num，线程池的数量，默认值为8
     * @param max_requests，允许最大的请求数量，默认值为10000
     */
    Thread_Pool(int thread_num = 8, int max_requests = 10000);

    /**
     * @brief 析构函数
     */
    ~Thread_Pool();

    /**
     * @brief 向请求队列中添加任务
     * @param task，请求加入队列的任务
     */
    bool add_task(Task& task);

private:
    /**
     * @brief 线程的执行业务函数
     * @param args，主线程给子线程传递的参数
     * @return void*，返回线程的状态
     */
    static void* Work_Callback(void* args);

    /**
     * @brief 线程池开始执行的逻辑函数
     * @return void类型
     */
    void run();

private:
    /**
     * @brief 线程的数量
     */
    int _thread_num;

    /**
     * @brief 线程池数组，大小为 _thread_num
     */
    pthread_t* _threads;

    /**
     * @brief 请求队列中最多允许的，等待处理的请求数量
     */
    int _max_requests;

    /**
     * @brief 请求队列
     */
    std::list<Task> _work_queue;

    /**
     * @brief 互斥锁，需要保证请求队列的同步
     */
    Locker _queue_locker;

    /**
     * @brief 信号量，用来判断是否有任务需要处理
     */
    Sem Work_num;

    /**
     * @brief 是否结束线程
     */
    bool _stop;
};

#endif
