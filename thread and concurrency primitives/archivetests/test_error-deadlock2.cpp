/*
 * Checking behavior of deadlock with cv
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex mutex1;
cv cv1;

int zero = 0; 

void threadb(void *a)
{
    char *id = (char *) a;

    mutex1.lock();

    
    while(zero != 1)
    {
        cout << id << " waiting for zero = 1" << endl;
        cv1.wait(mutex1);
    }
    cout << id << " zero = 1! Wait..." << endl;

    mutex1.unlock();
}

void threada(void *a)
{
    char *id = (char *) a;

    mutex1.lock();

    while(zero != 2)
    {
        cout << id << " waiting for zero = 2" << endl;
        cv1.wait(mutex1);
    }   
    cout << id << " zero = 2! Wait..." << endl;
    
    mutex1.unlock();
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