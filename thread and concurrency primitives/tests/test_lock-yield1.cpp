/*
 * Checking lock & cv (signal, wait) - 3 threads. Test broadcast.
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex mutex1;

void threadb(void *a)
{
    char *id = (char *) a;

    mutex1.lock();

    cout << id << "got lock" << endl;

    mutex1.unlock();
}

void threada(void *a)
{
    char *id = (char *) a;

    mutex1.lock();
    cout << id << " got lock. yielding to b. " << endl;
    thread::yield();
    mutex1.unlock();
    cout << id << " released lock " << endl;
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