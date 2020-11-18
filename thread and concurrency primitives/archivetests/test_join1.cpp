/*
 * Checking behavior of join and lifetimes
 * Normal case join (called when both object and thread alive)
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex mutex1;

void threadfunc(void *a)
{
    char *id = (char *) a;

    mutex1.lock();
    cout << id << " executing " << endl;
    mutex1.unlock();
}

void parent(void *a)
{
    intptr_t arg = (intptr_t) a;

    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();

    if(true)
    {
        thread thread_a ( (thread_startfunc_t) threadfunc, (void *) "thread a");
    }
    thread thread_b ( (thread_startfunc_t) threadfunc, (void *) "thread b");
    
    cout << "calling join on thread_b" << endl;
    thread_b.join();
    
    cout << "parent finished " << endl;
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}