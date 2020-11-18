/*
 * Checking lock & cv (broadcast, wait) - 3 threads
 */
#include <iostream>

#include "thread.h"
#include "cv.h"
#include "mutex.h"

using std::cout;
using std::endl;

int num = 3;
bool hitZero = false;
mutex lck;
cv waiting;
cv waitingToFinish;

void minus(void *a)
{
    char* id = (char*) a;

    lck.lock();
    cout << id << endl;
    num--;
    if (num == 0) {
        hitZero = true;
        waiting.broadcast();
    } 
    lck.unlock();
}
void add(void *a)
{
    char* id = (char*) a;
    lck.lock();
    cout << id << endl;
    while (!hitZero) {
        waiting.wait(lck);
    }
    num++;
    if (num == 4) {
        waitingToFinish.signal();
    }
    lck.unlock();
}
void parent(void *input)
{
    cout << "parent called with arg " << input << endl;
    thread t1 ( (thread_startfunc_t) add, (void *) "add thread");
    thread t2 ( (thread_startfunc_t) minus, (void *) "minus thread");
    thread t3 ( (thread_startfunc_t) add, (void *) "add thread");
    thread t4 ( (thread_startfunc_t) add, (void *) "add thread");
    thread t5 ( (thread_startfunc_t) minus, (void *) "minus thread");
    thread t6 ( (thread_startfunc_t) minus, (void *) "minus thread");
    thread t7 ( (thread_startfunc_t) add, (void *) "add thread");
    lck.lock();
    while (num != 4) {
        waitingToFinish.wait(lck);
    }
    lck.unlock();
    cout << "done" << endl;
}

int main() {
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
};