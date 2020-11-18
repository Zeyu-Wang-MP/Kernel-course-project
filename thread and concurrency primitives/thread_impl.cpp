#include "thread_impl.h"
#include "cpu.h"
#include "cpu_impl.h"
#include "ucontext_wrapper.h"
#include <ucontext.h>
#include <cstdio>
using std::queue;

// interrupt disabled when call this function
void thread::impl::execute_user_function(
    thread_startfunc_t func, 
    void* arg, 
    queue<ucontext_wrapper_t*>* waiting_queue_ptr,
    thread::impl* threadimpl_object_ptr, 
    bool* whether_thread_die){

    assert_interrupts_disabled();
    cpu::self()->impl_ptr->release_last_context();
    //printf("DEBUG: thread::execute_user_function: start to execute user code\n");
    cpu::interrupt_enable();
    func(arg);
    assert_interrupts_enabled(); 
    cpu::interrupt_disable();
    // if now, original thread object is still alive
    if(!*whether_thread_die){
        //printf("DEBUG: thread::execute_user_function: user code done! thread object still alive\n");
        threadimpl_object_ptr->whether_execute_done = true;
    }
    else{
        //printf("DEBUG: thread::execute_user_function: user code done! thread object is died\n");
    }
    // user thread done, we need to deal with waiting queue(remember to release memory)
    if(!waiting_queue_ptr->empty()){
        //printf("DEBUG: thread::execute_user_function: some threads are waiting for this execution finishing\n");
        // move all contexts to ready queue
        while(!waiting_queue_ptr->empty()){
            cpu::self()->impl_ptr->ready_queue.push(waiting_queue_ptr->front());
            waiting_queue_ptr->pop();
        }
    }
    // release memory we use
    delete waiting_queue_ptr;
    delete whether_thread_die;

    // after thread finish, can't delete thread stack since currently CPU are using 
    // this stack
    cpu::self()->impl_ptr->last_context = cpu::self()->impl_ptr->curr_context;
    // only use kernel context to release memory when this is the last runnable thread
    if(cpu::self()->impl_ptr->ready_queue.empty()){
        setcontext(&cpu::self()->impl_ptr->kernel_context);
    }
    // if we still have runnable threads, we let these threads to release memory of this thread
    else{
        cpu::self()->impl_ptr->curr_context = cpu::self()->impl_ptr->ready_queue.front();
        cpu::self()->impl_ptr->ready_queue.pop();
        setcontext(&cpu::self()->impl_ptr->curr_context->context);
    } 
}

// interrupt enabled when call this function
thread::impl::impl(thread_startfunc_t func, void* arg):
    waiting_queue_ptr(nullptr), 
    whether_thread_die(nullptr),
    whether_execute_done(false){

    assert_interrupts_enabled();
    cpu::interrupt_disable();
    // create new context
    ucontext_wrapper_t* new_context = new(std::nothrow) ucontext_wrapper_t();
    if(new_context == nullptr){
        cpu::interrupt_enable();
        throw std::bad_alloc();
    }
    
    char* stack = new(std::nothrow) char[STACK_SIZE];
    if(stack == nullptr){
        cpu::interrupt_enable();
        throw std::bad_alloc();
    }
    
    new_context->context.uc_stack.ss_sp = stack;
    new_context->context.uc_stack.ss_size = STACK_SIZE;
    new_context->context.uc_stack.ss_flags = 0;
    new_context->context.uc_link = nullptr;
    // record the stack beginning
    new_context->stack_top = stack;

    // since thread executing and thread object lifetime is async, we have to use new
    whether_thread_die = new bool(false);
    waiting_queue_ptr = new(std::nothrow) queue<ucontext_wrapper_t*>;
    if(waiting_queue_ptr == nullptr){
        cpu::interrupt_enable();
        throw std::bad_alloc();
    }
    makecontext(
        &new_context->context, 
        (void (*)(void))thread::impl::execute_user_function, 
        5, func, arg, waiting_queue_ptr, this, whether_thread_die);
    
    //printf("DEBUG: thread ctor: this is a user created thread\n");
    cpu::impl::ready_queue.push(new_context);
    cpu::interrupt_enable();
}

// interrupt enabled when call this function
// this function only works before thread object is died
void thread::impl::join_impl(){
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    
    // if the execution of this thread is already done
    if(this->whether_execute_done){
        //printf("DEBUG: thread::join: target thread is done\n");
        cpu::interrupt_enable();
        return;
    }
    else{
        //printf("DEBUG: thread::join: target thread is executing, you have to wait\n");
    }
    // if ready queue is empty
    if(cpu::self()->impl_ptr->ready_queue.empty()){
        cpu::interrupt_enable_suspend();
    }
    // else we run next thread in ready queue
    else{
        // add current thread to waiting queue
        waiting_queue_ptr->push(cpu::self()->impl_ptr->curr_context);
        cpu::self()->impl_ptr->curr_context = cpu::self()->impl_ptr->ready_queue.front();
        cpu::self()->impl_ptr->ready_queue.pop();
        assert_interrupts_disabled();
        swapcontext(&waiting_queue_ptr->back()->context, &cpu::self()->impl_ptr->curr_context->context);
        assert_interrupts_disabled();
        cpu::self()->impl_ptr->release_last_context();
        cpu::interrupt_enable();
    }
}
// interrupt is enabled when call this function
thread::impl::~impl(){
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    //printf("DEBUG: thread::impl: thread dtor\n");
    if(!this->whether_execute_done){
        *whether_thread_die = true;
    }
    cpu::interrupt_enable();
}
