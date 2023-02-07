#ifndef __THREAD_H__
#define __THREAD_H__
#include <boost/noncopyable.hpp>
#include <string>
#include <functional>
#include "atomic.h"
namespace ekko{
using std::string;

class Thread : boost::noncopyable{
public:
    typedef std::function<void(void)> ThreadFunc;
    explicit Thread(ThreadFunc, const string& name = string());
    ~Thread(); /*please use smart pointer*/

    void start(); /*cannot be called by more than one threads*/
    int join(); /*cannot be called by more than one threads*/
    pid_t tid() const { return tid_;}
    const string& name() const { return name_;}
    bool isStarted() const { return started_;}
    bool isJoined() const { return joined_;}

    static int numCreated() { return num_of_threads_.get();}
private:
    bool started_;
    bool joined_;
    pid_t tid_;
    pthread_t pthreadId_;
    ThreadFunc func_;
    string name_;
    static AtomicInt32 num_of_threads_;
};

extern __thread Thread* t_this_thread;
}
#endif