/*
 * Checking deadlock behavior with join. 
 * Calling join when: (thread object destroyed, thread still executes)
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex mutex1;
thread* threadbpointer;

void threadfunc(void *a)
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

    thread thread_b ( (thread_startfunc_t) threadfunc, (void *) "thread b");
    
    // Set global thread pointer to b
    threadbpointer = &thread_b;


    
    cout << "parent finished " << endl;
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}