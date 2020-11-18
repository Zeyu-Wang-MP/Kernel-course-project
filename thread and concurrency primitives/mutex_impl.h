#ifndef _MUTEX_IMPL_H
#define _MUTEX_IMPL_H

#include "ucontext_wrapper.h"
#include "mutex.h"
#include <queue>

class mutex::impl {
public:
    impl(mutex* mutex_ptr);
    ~impl();
    void lockImpl();
    void unlockImpl();
    void cvUnlock();
    ucontext_wrapper_t* mutexHolder;
private:
    void unlockSource();
    
    bool free;
    std::queue<ucontext_wrapper_t*> waitingMutex;
    mutex* original_mutex_ptr;
};

#endif