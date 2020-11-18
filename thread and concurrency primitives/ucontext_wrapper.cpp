#include "ucontext_wrapper.h"
#include "mutex_impl.h"

ucontext_wrapper_t::~ucontext_wrapper_t(){
    delete[] stack_top;
    
    for(mutex* each : current_holding_mutex){
        each->impl_ptr->mutexHolder = nullptr;
    }
}