#ifndef _UCONTEXT_WRAPPER_H_
#define _UCONTEXT_WRAPPER_H_

#include "mutex.h"
#include <unordered_set>
#include <ucontext.h>

struct ucontext_wrapper_t{
    ucontext_t context;
    char* stack_top;
    std::unordered_set<mutex*> current_holding_mutex;
    ucontext_wrapper_t():stack_top(nullptr){}
    ~ucontext_wrapper_t();

    //delete move and copy ctor and assign operator
    ucontext_wrapper_t(const ucontext_wrapper_t&) = delete;
    ucontext_wrapper_t(ucontext_wrapper_t&&) = delete;
    ucontext_wrapper_t& operator=(const ucontext_wrapper_t&) = delete;
    ucontext_wrapper_t& operator=(ucontext_wrapper_t&&) = delete;
};


#endif