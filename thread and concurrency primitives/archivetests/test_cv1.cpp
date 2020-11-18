/*
 * Test for signal on empty waiting queue. 
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

int g = 0;

mutex mutex1;
cv cv1;

void wait_g(void *a)
{
    char *id = (char *) a;

    mutex1.lock();
    cout << id << " called " << endl;
    cout << id << " waiting on g to be 1 " << endl;
    

    while(g != 1)
    {
        cv1.wait(mutex1);
    }

    cout << id << " now saw 1 " << endl;


    mutex1.unlock();
}

void change_g(void *a)
{
    char *id = (char *) a;

    mutex1.lock();
    cout << id << " called " << endl;
    g = 1; 
    cout << id << " set g to 1 " << endl;
    
    cout << id << " broadcasting cv1 " << endl;
    cv1.broadcast();
    cv1.broadcast();
    thread::yield();
    cv1.signal();
    thread::yield();
    cv1.signal();
    mutex1.unlock();

}

void parent(void *a)
{
    intptr_t arg = (intptr_t) a;

    mutex1.lock();
    cout << "parent called with arg " << arg << endl;
    mutex1.unlock();

    thread t1 ( (thread_startfunc_t) wait_g, (void *) "wait_g thread 1");
    thread t2 ( (thread_startfunc_t) wait_g, (void *) "wait_g thread 2");
    thread t3 ( (thread_startfunc_t) change_g, (void *) "change_g thread");
    
    cout << "parent finished " << endl;
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
}