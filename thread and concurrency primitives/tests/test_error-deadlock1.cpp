/*
 * Checking behavior of deadlock with mutex
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex mutex1;
mutex mutex2;

void threadb(void *a)
{
    char *id = (char *) a;

    mutex2.lock();
    cout << id << " got lock 2. Attempt to get lock 1" << endl;
    
    mutex1.lock();
    cout << id << " got lock 1. Should not have happened..." << endl;
    
    mutex2.unlock();
    mutex1.unlock();
}

void threada(void *a)
{
    char *id = (char *) a;

    mutex1.lock();
    cout << id << " got lock 1. yielding to b. " << endl;
    thread::yield();

    mutex2.lock();
    cout << id << " got lock 2. Should not have happened... " << endl;
    
    
    mutex1.unlock();
    mutex2.unlock();
}

void parent(void *a)
{
    intptr_t arg = (intptr_t) a;

    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();

    thread t1 ( (thread_startfunc_t) threada, (void *) "thread a");
    thread t2 ( (thread_startfunc_t) threadb, (void *) "thread b");
    
    cout << "parent finished " << endl;
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}