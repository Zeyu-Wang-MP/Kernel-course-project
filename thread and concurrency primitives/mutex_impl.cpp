#include "mutex_impl.h"
#include "cpu_impl.h"
#include <queue>
#include <stdexcept>

mutex::impl::impl(mutex* mutex_ptr):mutexHolder(nullptr), free(true), original_mutex_ptr(mutex_ptr){}

mutex::impl::~impl() {
}
void mutex::impl::lockImpl() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    // if other one holds this mutex
    if (!free) {
        waitingMutex.push(cpu::self()->impl_ptr->curr_context);
        // if the ready queue has other thread to run
        if (!cpu::self()->impl_ptr->ready_queue.empty())
        {
            cpu::self()->impl_ptr->curr_context = cpu::self()->impl_ptr->ready_queue.front();
            cpu::self()->impl_ptr->ready_queue.pop();
            swapcontext(&waitingMutex.back()->context, &cpu::self()->impl_ptr->curr_context->context);
            assert_interrupts_disabled();
            // after we come back, we check if we need to release last context's memory
            cpu::self()->impl_ptr->release_last_context();
        }
        else {
            cpu::interrupt_enable_suspend();
        }
        assert_interrupts_disabled();
    }
    else {
        free = false;
        
        //remember mutex holder
        mutexHolder = cpu::self()->impl_ptr->curr_context;
        // this thread's context need add current mutex to its set
        mutexHolder->current_holding_mutex.insert(original_mutex_ptr);
    };
    cpu::interrupt_enable();

}
void mutex::impl::unlockImpl() {
    assert_interrupts_enabled();
    cpu::interrupt_disable();
    unlockSource();
    cpu::interrupt_enable();
}

//For cv use only
void mutex::impl::cvUnlock() {
    assert_interrupts_disabled();
    unlockSource();
    assert_interrupts_disabled();
}

void mutex::impl::unlockSource() {
    assert_interrupts_disabled();
    // See if unlocker thread is the mutex holder
    if (cpu::self()->impl_ptr->curr_context != mutexHolder)
    {
        //throwing error returns to user code I think.
        cpu::interrupt_enable();
        throw std::runtime_error("Current thread does not hold mutex and cannot unlock.");
    }
    free = true;
    mutexHolder->current_holding_mutex.erase(original_mutex_ptr);
    // lock handoff
    if (!waitingMutex.empty()) {
        cpu::self()->impl_ptr->ready_queue.push(waitingMutex.front());
        waitingMutex.pop();
        
        mutexHolder = cpu::self()->impl_ptr->ready_queue.back();
        free = false;
        mutexHolder->current_holding_mutex.insert(original_mutex_ptr);
    }  
    else{
        mutexHolder = nullptr;
    }
    assert_interrupts_disabled();
}