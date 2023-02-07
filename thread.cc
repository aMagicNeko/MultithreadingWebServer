#include "thread.h"
#include <assert.h>
namespace ekko{
__thread Thread *t_this_thread;

void* startThread(void* ptr) {
    t_this_thread = static_cast<Thread*>(ptr);
    return NULL;
}

Thread::Thread(ThreadFunc f, const string& name) :
    started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(move(f)),
    name_(name)
{
    int num = num_of_threads_.addAndGet(1);
    if (name_.empty()) {
        char buff[32];
        snprintf(buff, sizeof(buff), "Thread%d", num);
        name_ = buff;
    }
}

Thread::~Thread() {
    if (started_ && !joined_)
        pthread_detach(pthreadId_);
}

void Thread::start() {
    assert(!started_);
    started_ = true;
    if (pthread_create(&pthreadId_, NULL, startThread, this)) {
        started_ = false;
        LOG_SYSFATAL << "Failed in pthread_create";
    }
}

int Thread::join() {
    assert(started_ & !joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}

}