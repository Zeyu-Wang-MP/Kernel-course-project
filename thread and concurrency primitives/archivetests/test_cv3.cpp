/*
 * Signaling while not holding the lock. 
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

mutex mutex1;
cv waiting;
int num = 0;

void threadfunc(void *a)
{
    cout << a << " started" << endl;
    mutex1.lock();
    cout << a << " acquired lock" << endl;
    num++;
    if (num < 3) {
        waiting.wait(mutex1);
    }
    cout << a << " exited" << endl;
    mutex1.unlock();
}
void badFunc(void *a)
{
    cout << a << " started" << endl;
    waiting.broadcast();
}

void parent(void *a)
{

    cout << a << " started" << endl;
    thread t1 ( (thread_startfunc_t) threadfunc, (void *) "threadfunc");
    thread t2 ( (thread_startfunc_t) threadfunc, (void *) "threadfunc");
    thread t4 ( (thread_startfunc_t) badFunc, (void *) "badFunc");
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) "parentFunc", false, false, 0);
}