/*
 * Checking lock & cv (signal, wait) - 2 threads
 */
#include <iostream>

#include "thread.h"
#include "cv.h"
#include "mutex.h"

using std::cout;
using std::endl;

int childDone = 0;
mutex lck;
cv waiting;

void child(void *a)
{
    char *id = (char *) a;
    lck.lock();
    cout << id << endl;
    childDone = 1;
    waiting.signal();
    lck.unlock();
}

void parent(void *input)
{
    cout << "parent called with arg " << input << endl;
    thread t1 ( (thread_startfunc_t) child, (void *) "child thread");
    lck.lock();
    while (!childDone) {
        cout << "parent wait" << endl;
        waiting.wait(lck);
    }
    cout << "finished" << endl;
    lck.unlock();
}

int main() {
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
};