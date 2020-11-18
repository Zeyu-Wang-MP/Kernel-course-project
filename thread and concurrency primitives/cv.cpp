/*
 * cv.cpp
 */
#include "cv.h"
#include "cv_impl.h"

//allocate impl
cv::cv() 
{
    impl_ptr = new(std::nothrow) impl;
    if(impl_ptr == nullptr){
        throw std::bad_alloc();
    }
}

//delete impl
cv::~cv() 
{
    delete impl_ptr;
}

//Call wait impl
void cv::wait(mutex& mutex_obj) 
{
    impl_ptr->wait_impl(mutex_obj);
}

//Call signal impl
void cv::signal() 
{
    impl_ptr->signal_impl();
}

//Call broadcast impl
void cv::broadcast() 
{
    impl_ptr->broadcast_impl();
}