#ifndef __ATOMIC_H__
#define __ATOMIC_H__
#include <boost/noncopyable.hpp>
namespace ekko {
template<class T>
class AtomicInterger : boost::noncopyable {
public:
    AtomicInterger() : value_(0){}

    T get() { return __sync_val_compare_and_swap(&value_, 0, 0);}
    T getAndAdd(T x) { return __sync_fetch_and_add(&value_, x);}
    T addAndGet(T x) { return getAndAdd(x) + x;}
    void add(T x) { getAndAdd(x);}
    T getAndSet(T x) { _sync_lock_test_and_set(&value_, x);}
private:
    T value_;
};

typedef AtomicInterger<int> AtomicInt32;
typedef AtomicInterger<long long> AtomicInt64;
typedef AtomicInterger<short> AtomicInt16;
}
#endif