/**
 * @file threadpool.cpp
 * @brief 线程池封装
 * @author 刘治学
 */

#include "threadpool.h"

// 不放这个会报错，两种解决方案：模板类的实现在头文件中进行；在源文件中显示实例化我们用到的模板类
template class Thread_Pool<Http_Conn>;

template <class Task>
Thread_Pool<Task>::Thread_Pool(int thread_num, int max_requests)
    : _thread_num(thread_num), _max_requests(max_requests), _stop(false), _threads(nullptr) {
    if (thread_num <= 0 || max_requests <= 0)  // 传入的参数数据不对
        throw std::exception();

    // 动态创建线程池数组
    _threads = new pthread_t[thread_num];
    if (!_threads)
        throw std::exception();

    // 创建指定数量的线程并且设置线程分离
    for (int i = 0; i < thread_num; ++i) {
        // 创建线程
        printf("create the %d thread.\n", i);
        // Work_Callback回调函数我们定义成static，这样就没办法访问类内的成员，所以参数我把this指针传进来
        if (0 != pthread_create(&_threads[i], nullptr, Work_Callback, this)) {
            delete[] _threads;
            throw std::exception();
        }

        // 设置线程分离
        if (0 != pthread_detach(_threads[i])) {
            delete[] _threads;
            throw std::exception();
        }
    }
}

template <class Task>
Thread_Pool<Task>::~Thread_Pool() {
    delete[] _threads;
    _stop = true;
}

template <class Task>
bool Thread_Pool<Task>::add_task(Task& task) {
    // 向请求队列中加入数据，需要保证同步
    _queue_locker.lock();

    if (_work_queue.size() >= _max_requests) {
        // 再加就超出最大量了
        _queue_locker.unlock();
        return false;
    }

    _work_queue.push_back(task);
    _queue_locker.unlock();
    Work_num.post();  // 信号量加加，代表有新任务进来了

    return true;
}

template <class Task>
void* Thread_Pool<Task>::Work_Callback(void* args) {
    Thread_Pool* pool = (Thread_Pool*)args;

    pool->run();

    return pool;
}

template <class Task>
void Thread_Pool<Task>::run() {
    // 如果不停止就一直跑
    while (!_stop) {
        Work_num.wait();  // PV操作，需要看有没有任务可以做，然后信号量减减

        _queue_locker.lock();
        // if (_work_queue.empty()) {  // 如果队列为空重新循环，前面有信号量保证，不可能为空
        //     _queue_locker.unlock();
        //     continue;
        // }

        // 队列不为空肯定能拿到
        Task& task = _work_queue.front();
        _work_queue.pop_front();

        _queue_locker.unlock();

        task.process();  // 任务逻辑
    }
}
