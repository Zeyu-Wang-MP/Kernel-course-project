#ifndef _THREAD_IMPL_H_
#define _THREAD_IMPL_H_

#include "thread.h"
#include "ucontext_wrapper.h"
#include <queue>


class thread::impl{
public:
    
    // queue for other thread waiting for this thread to finish executing, 
    // not waiting for this thread object to die!!
    std::queue<ucontext_wrapper_t*>* waiting_queue_ptr;

    bool* whether_thread_die;
    bool whether_execute_done;

    // this function start with interrupt disabled
    static void execute_user_function(
        thread_startfunc_t func, 
        void* arg, 
        std::queue<ucontext_wrapper_t*>* waiting_queue_ptr,
        thread::impl* threadimpl_object_ptr,
        bool* whether_thread_die
    );

    //interrupt enabled when call this function
    impl(thread_startfunc_t func, void* arg);

    ~impl();
    //interrupt enabled when call this function
    void join_impl();
};

#endif