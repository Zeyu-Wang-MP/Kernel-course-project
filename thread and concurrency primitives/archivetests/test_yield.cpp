/*
 * Yield when only one thread is running. 
 */

#include <iostream>
#include <cstdlib>
#include "thread.h"

using std::cout;
using std::endl;

void parent(void *a)
{
    cout << (char *)a << " started" << endl;
    thread::yield();
    cout << "Parent thread should still run after yield" << endl;
}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) "parentFunc", false, false, 0);
}