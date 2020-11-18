/*
 * mutex.cpp
 */
#include "mutex.h"
#include "mutex_impl.h"


mutex::mutex() {
    impl_ptr = new(std::nothrow) impl(this);
    if(impl_ptr == nullptr){
        throw std::bad_alloc();
    }
}
mutex::~mutex() {
    delete impl_ptr;
}
void mutex::lock() {
    impl_ptr->lockImpl();
}
void mutex::unlock() {
    impl_ptr->unlockImpl();
}

