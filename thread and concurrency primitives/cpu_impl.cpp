#include"cpu_impl.h"
#include"thread_impl.h"
#include<cstdio>
using std::queue;

// initialize static member
queue<ucontext_wrapper_t*> cpu::impl::ready_queue = queue<ucontext_wrapper_t*>();


cpu::impl::impl():curr_context(nullptr), last_context(nullptr){
    kernel_context.uc_stack.ss_sp = kernel_stack;
    kernel_context.uc_stack.ss_size = STACK_SIZE;
    kernel_context.uc_stack.ss_flags = 0;
    kernel_context.uc_link = nullptr;
    makecontext(&kernel_context, (void (*)(void))handle_thread_finish, 0);
}

// interrupt is disabled when call this function
// only call this function when the current thread is the last runnable thread
void cpu::impl::handle_thread_finish(){
    assert_interrupts_disabled();
    //printf("DEBUG: kernel code: one thread done\n");
    delete cpu::self()->impl_ptr->last_context; 
    cpu::self()->impl_ptr->curr_context = nullptr;
    cpu::self()->impl_ptr->last_context = nullptr;
    
    cpu::interrupt_enable_suspend();
}

// interrupt is disabled when call this function
void cpu::impl::release_last_context(){
    assert_interrupts_disabled();
    if(this->last_context != nullptr){
        delete this->last_context;
        this->last_context = nullptr;
    }
}

// interrupt is disabled when call this function
void cpu::impl::init_impl(thread_startfunc_t func, void* arg){
    assert_interrupts_disabled();
    if(func == nullptr){
        cpu::interrupt_enable_suspend();
    }

    // need to set interrupt vector table
    cpu::self()->interrupt_vector_table[cpu::TIMER] = interrupt_handler_TIMER;
    cpu::self()->interrupt_vector_table[cpu::IPI] = interrupt_handler_IPI;

    // since 1 CPU, we create main thread and execute it
    //printf("DEBUG: cpu::init: create and run first thread\n");
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
    // since we don't have thread object for first thread, we assume it's died
    // no one can call join on first thread
    bool* whether_thread_die = new bool(true);
    queue<ucontext_wrapper_t*>* waiting_queue_ptr = new(std::nothrow) queue<ucontext_wrapper_t*>;
    if(waiting_queue_ptr == nullptr){
        cpu::interrupt_enable();
        throw std::bad_alloc();
    }
    makecontext(
        &new_context->context, 
        (void (*)(void))thread::impl::execute_user_function, 
        5, func, arg, waiting_queue_ptr, nullptr, whether_thread_die);
    cpu::self()->impl_ptr->curr_context = new_context;
    setcontext(&new_context->context);  
}

// when handler get called, remember interrupt is still enabled
void cpu::impl::interrupt_handler_TIMER(){
    assert_interrupts_enabled();
    thread::yield();
}

// when handler get called, remember interrupt is still enabled    
void cpu::impl::interrupt_handler_IPI(){
    // do nothing since we only have 1 CPU
}