#include "cpu.h"
#include "cpu_impl.h"
#include "cv.h"
#include "cv_impl.h"
#include "mutex.h"
#include "mutex_impl.h"
#include <queue>
#include <cstdio>

using std::queue;

cv::impl::impl() 
{

}

cv::impl::~impl() 
{

}

void cv::impl::wait_impl(mutex& mutex_obj)
{
    assert_interrupts_enabled();
    //printf("DEBUG: entering wait_impl\n");
    cpu::interrupt_disable();
    mutex_obj.impl_ptr->cvUnlock();
    

    //Push currently running thread onto cv waiting queue
    cv_queue.push(cpu::self()->impl_ptr->curr_context);
    
    //Assign new running thread to cpu
    if (cpu::self()->impl_ptr->ready_queue.size() != 0)
    {
        cpu::self()->impl_ptr->curr_context = cpu::self()->impl_ptr->ready_queue.front();
        cpu::self()->impl_ptr->ready_queue.pop();
    }
    else
    {
        //No other threads in ready queue. Sleep. Deadlocked.
        assert_interrupts_disabled();
        cpu::interrupt_enable_suspend();
    }


    //Sleep - save current context and swap to current running thread
    //printf("DEBUG: exiting wait_impl\n");
    assert_interrupts_disabled();
    swapcontext(&cv_queue.back()->context, &cpu::self()->impl_ptr->curr_context->context);
    assert_interrupts_disabled();
    cpu::self()->impl_ptr->release_last_context();

    //TODO: Ordering of this?
    cpu::interrupt_enable();
    mutex_obj.lock();
    
    //Run user code after locked. 
}

void cv::impl::signal_impl()
{
    assert_interrupts_enabled();
    cpu::interrupt_disable();

    //printf("%s%li%s","DEBUG: before Signal RQ size: ", cpu::self()->impl_ptr->ready_queue.size(), "\n");
    //printf("%s%li%s","DEBUG: before Signal cvQ size: ", cv_queue.size(), "\n");
    //Put front element of cv_queue to back of ready_queue
    if(!cv_queue.empty())
    {
    
        cpu::self()->impl_ptr->ready_queue.push(cv_queue.front());
        cv_queue.pop();
    
    }
    //printf("%s%li%s","DEBUG: after Signal RQ size: ", cpu::self()->impl_ptr->ready_queue.size(), "\n");
    //printf("%s%li%s","DEBUG: after Signal cvQ size: ", cv_queue.size(), "\n");

    cpu::interrupt_enable();
    assert_interrupts_enabled();
    
    //Return to user code...
}

void cv::impl::broadcast_impl()
{
    assert_interrupts_enabled();
    cpu::interrupt_disable();

    //Put all elements of cv_queue to back of ready_queue
    //printf("%s%li%s","DEBUG: before Broadcast RQ size: ", cpu::self()->impl_ptr->ready_queue.size(), "\n");
    //printf("%s%li%s","DEBUG: before Broadcast cvQ size: ", cv_queue.size(), "\n");
    while(!cv_queue.empty())
    {
        cpu::self()->impl_ptr->ready_queue.push(cv_queue.front());
        cv_queue.pop();
    }
    //printf("%s%li%s","DEBUG: After Broadcast RQ size: ", cpu::self()->impl_ptr->ready_queue.size(), "\n");
    //printf("%s%li%s","DEBUG: After Broadcast cvQ size: ", cv_queue.size(), "\n");

    
    cpu::interrupt_enable();
    assert_interrupts_enabled();
    
    //Return to user code...

}

//Helper
queue<ucontext_wrapper_t*>* CPUReadyQ()
{
    return &cpu::self()->impl_ptr->ready_queue;
}