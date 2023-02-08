#ifndef __LOCK_H__
#define __LOCK_H__
#include <pthread.h>
#include <assert.h>
#include "thread.h"
#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})
namespace ekko{
class Condition;
class Mutex {
public:
    Mutex() : tid_(0) {
        MCHECK(pthread_mutex_init(&mutex_, 0));
    }

    ~Mutex() {
        assert(tid_ == 0);
        MCHECK(pthread_mutex_destroy(&mutex_));
    }

    bool isLockedByThisThread() const {
        return tid_ == t_this_thread->tid();
    }
    
    void lock() {
        MCHECK(pthread_mutex_lock(&mutex_));
        tid_ = t_this_thread->tid();
    }

    void unlock() {
        tid_ = 0;
        MCHECK(pthread_mutex_unlock(&mutex_));
    }

    int trylock() {
        int ret;
        if (pthread_mutex_trylock(&mutex_) == 0)
            tid_ = t_this_thread->tid();
        return ret;
    }
private:
    friend class Condition;
    pthread_mutex_t mutex_;
    pid_t tid_;
};

class MutexGuard {
public:
    MutexGuard(Mutex &m) : mutex_(m) { mutex_.lock();}

    ~MutexGuard() { mutex_.unlock();}
private:
    Mutex& mutex_;
};

class Condition : boost::noncopyable{
public:
    Condition(Mutex& m) :mutex_(m) {
        MCHECK(pthread_cond_init(&pcond_));
    }

    ~Condition() {
        MCHECK(pthread_cond_destroy(&pcond_));
    }
    /* must unlock the mutex after using*/
    void wait() {
        if (!mutex_.isLockedByThisThread())
            mutex_.lock();
        pthread_cond_wait(&pcond_, &mutex_.mutex_);
    }

    void signal() {
        mutex_.lock();
        MCHECK(pthread_cond_signal(&pcond_));
        mutex_.unlock();
    }

    void broadcast() {
        MCHECK(pthread_cond_broadcast(&pcond_));
    }
private:
    pthread_cond_t pcond_;
    Mutex &mutex_;
};


}
#endif