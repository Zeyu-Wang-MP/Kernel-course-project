/*
 * cpu.cpp -- thread library implementation for cpu boot
 */

#include "thread.h"
#include "cpu.h"
#include "cpu_impl.h"
#include "thread_impl.h"
#include "ucontext_wrapper.h"
#include <ucontext.h>
#include <cstdio>
#include <deque>

using std::deque;
using std::string;

// interrupt is enabled when call this function
thread::thread(thread_startfunc_t func, void* arg){
    this->impl_ptr = new(std::nothrow) thread::impl(func, arg);
    if(impl_ptr == nullptr){
        throw std::bad_alloc();
    }
}

thread::~thread(){
    delete this->impl_ptr;
}
// interrupt is enabled when call this function
void thread::yield(){
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    // if ready queue doesn't have context, just continue to execute current thread
    if(cpu::self()->impl_ptr->ready_queue.empty()){
        //printf("DEBUG: thread::yield: ready queue is empty, continue to execute\n");
        cpu::interrupt_enable();
        return;
    }
    // if ready queue has a context
    else{
        //printf("DEBUG: thread::yield: switch to another thread in ready queue\n");
        // save old context
        cpu::self()->impl_ptr->ready_queue.push(cpu::self()->impl_ptr->curr_context);
        // update curr context to new context
        cpu::self()->impl_ptr->curr_context = cpu::self()->impl_ptr->ready_queue.front();
        cpu::self()->impl_ptr->ready_queue.pop();
        
        assert_interrupts_disabled();
        swapcontext(&cpu::self()->impl_ptr->ready_queue.back()->context, &cpu::self()->impl_ptr->curr_context->context);
        assert_interrupts_disabled();
        cpu::self()->impl_ptr->release_last_context(); 
        cpu::interrupt_enable();
    }
}

string thread::swap3(const string& input_string){
    assert_interrupts_enabled();
    cpu::self()->interrupt_disable();

    // if this is not the third thread calling this function
    if(cpu::self()->impl_ptr->swap3_waiting_queue.size() < 2){
        if(cpu::self()->impl_ptr->ready_queue.empty()){
            cpu::interrupt_enable_suspend();
        }
        
        //else
        // if this thread is in a new phase
        if(cpu::self()->impl_ptr->swap3_waiting_queue.empty()){
            cpu::self()->impl_ptr->swap3_memory.push(deque<string>());
        }
        // add its string to the latest deque
        cpu::self()->impl_ptr->swap3_memory.back().push_back(input_string);
        // add this thread to waiting queue
        cpu::self()->impl_ptr->swap3_waiting_queue.push(cpu::self()->impl_ptr->curr_context);

        // choose new thread to run
        cpu::self()->impl_ptr->curr_context = cpu::self()->impl_ptr->ready_queue.front();
        cpu::self()->impl_ptr->ready_queue.pop();
        swapcontext(&cpu::self()->impl_ptr->swap3_waiting_queue.back()->context, &cpu::self()->impl_ptr->curr_context->context);
        assert_interrupts_disabled();
        // after we come back, need to release last thread's context if needed
        cpu::self()->impl_ptr->release_last_context();
        
        // get the string from the oldest deque
        string res = cpu::self()->impl_ptr->swap3_memory.front().front();
        cpu::self()->impl_ptr->swap3_memory.front().pop_front();

        // if we already done the commnuication of this phase, clear the deque we use
        if(cpu::self()->impl_ptr->swap3_memory.front().empty()){
            cpu::self()->impl_ptr->swap3_memory.pop();
        }

        cpu::interrupt_enable();
        return res;

    }
    // if this is the third thread calling this function
    else{
        // get the string from the latest deque
        string res = cpu::self()->impl_ptr->swap3_memory.back().back();
        cpu::self()->impl_ptr->swap3_memory.back().pop_back();
        // move 2 threads to ready queue
        while(!cpu::self()->impl_ptr->swap3_waiting_queue.empty()){
            cpu::self()->impl_ptr->ready_queue.push(cpu::self()->impl_ptr->swap3_waiting_queue.front());
            cpu::self()->impl_ptr->swap3_waiting_queue.pop();
        }
        // push its own string to the latest deque
        cpu::self()->impl_ptr->swap3_memory.back().push_front(input_string);

        cpu::interrupt_enable();
        return res;
    }
    
}

// interrupt is enabled when call this function
void thread::join(){
    this->impl_ptr->join_impl();
}
