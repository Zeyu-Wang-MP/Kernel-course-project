#ifndef _CPU_IMPL_H_
#define _CPU_IMPL_H_

#include"cpu.h"
#include"ucontext_wrapper.h"
#include"thread.h"
#include<ucontext.h>
#include<queue>
#include<deque>
#include<string>


class cpu::impl{
public:
    // current context in our data structure, not curr_context in CPU!!
    ucontext_wrapper_t* curr_context;
    static std::queue<ucontext_wrapper_t*> ready_queue;
    // set this pointer only when a thread finishes its execution
    ucontext_wrapper_t* last_context;
    // context for kernel code to handle a thread finish
    ucontext_t kernel_context;
    // stack for kernel code
    char kernel_stack[STACK_SIZE];
    std::queue<ucontext_wrapper_t*> swap3_waiting_queue;
    std::queue<std::deque<std::string> > swap3_memory;
    
    static void interrupt_handler_TIMER();
    static void interrupt_handler_IPI();

    // make kernel context
    impl();
    void init_impl(thread_startfunc_t func, void* arg);
    // get called when CPU get the kernel context
    static void handle_thread_finish();
    
    void release_last_context();
};




#endif