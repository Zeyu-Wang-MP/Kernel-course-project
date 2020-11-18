/*
 * Thread dying while holding the lock. 
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex mutex1;

void threadfunc(void *a)
{
    cout << a << " started" << endl;
    mutex1.lock();
    cout << a << " acquired lock" << endl;
    mutex1.unlock();
}
void badFunc(void *a)
{
    cout << a << " started" << endl;
    mutex1.lock();
    cout << a << " acquired lock" << endl;
}

void parent(void *a)
{

    cout << a << " started" << endl;
    thread t2 ( (thread_startfunc_t) badFunc, (void *) "badFunc");
    thread t1 ( (thread_startfunc_t) threadfunc, (void *) "threadfunc");
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) "parentFunc", false, false, 0);
}