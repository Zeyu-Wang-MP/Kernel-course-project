#include"cpu.h"
#include"cpu_impl.h"
#include<ucontext.h>
#include<queue>
#include<cstdio>

using std::queue;

// interrupt is disabled when call this function
void cpu::init(thread_startfunc_t func, void* arg){
    assert_interrupts_disabled();
    // it will cause memory leak, but we don't need to release it
    this->impl_ptr = new(std::nothrow) cpu::impl();
    if(impl_ptr == nullptr){
        cpu::interrupt_enable();
        throw std::bad_alloc();
    }
    this->impl_ptr->init_impl(func, arg);
}