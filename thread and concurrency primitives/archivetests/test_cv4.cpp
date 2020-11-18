/*
 * Calling wait on one cv with different mutexes.
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex mutex1;
mutex mutex2;
cv waiting;
int num = 0;

void threadfunc(void *a)
{
    cout << a << " started" << endl;
    mutex1.lock();
    waiting.wait(mutex1);
    cout << a << " wait done" << endl;
    mutex1.unlock();
}
void threadfuncTwo(void *a)
{
    cout << a << " started" << endl;
    mutex2.lock();
    while (num == 0) {
        waiting.wait(mutex2);
    }
    cout << a << " wait done" << endl;
    mutex2.unlock();
}
void releaseFunc(void *a)
{
    cout << a << " started" << endl;
    num++;
    waiting.signal();
}
void parent(void *a)
{
    cout << a << " started" << endl;
    thread t1 ( (thread_startfunc_t) threadfunc, (void *) "threadfunc");
    thread t2 ( (thread_startfunc_t) threadfuncTwo, (void *) "threadfuncTwo");
    thread t3 ( (thread_startfunc_t) releaseFunc, (void *) "releaseFunc");
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) "parentFunc", false, false, 0);
}