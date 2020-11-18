/*
 * blockthread blocks, but normalfunc hasn't executed currently.
 * Unintened behavior?
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex mutex1;
thread* threadbpointer;


void normalfunc(void *a)
{
    char *id = (char *) a;

    mutex1.lock();
    cout << id << " executing " << endl;
    mutex1.unlock();
    cout << id << " finished " << endl;
}

void blockfunc(void *a)
{
    char *id = (char *) a;

    mutex1.lock();
    cout << id << " executing " << endl;

    // Join with self
    threadbpointer->join();

    // Should not reach here.
    cout << id << " finished waiting for " << id << " to finish" << endl;
    cout << "Wait..." << endl; 
    mutex1.unlock();
}

void parent(void *a)
{
    intptr_t arg = (intptr_t) a;

    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();

    
    thread blockthread ( (thread_startfunc_t) blockfunc, (void *) "blockthread");
    thread normalthread ( (thread_startfunc_t) normalfunc, (void *) "normalthread");

    
    // Set global thread pointer to blockthread
    threadbpointer = &blockthread;

    // Must yield here, otherwise parent goes out of scope
    // and thread_b object gets destroyed. 
    thread::yield();
    
    cout << "parent finished " << endl;
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}