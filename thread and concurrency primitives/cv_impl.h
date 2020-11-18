#ifndef _THREAD_IMPL_H_
#define _THREAD_IMPL_H_

#include "cv.h"
#include "ucontext_wrapper.h"
#include "mutex.h"
#include <queue>


class cv::impl
{
    public:
    //Constructor/Destructor
    impl();
    ~impl();

    //Wait implementation
    void wait_impl(mutex&);

    //Signal implementation
    void signal_impl();

    //Broadcast implementation
    void broadcast_impl();

    private:    
    std::queue<ucontext_wrapper_t*> cv_queue;


};

#endif