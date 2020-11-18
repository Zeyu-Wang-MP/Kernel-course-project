/*
 * General case
 */
#include <iostream>

#include "thread.h"

using std::cout;
using std::endl;

int g = 0;

void loop(void *a)
{
    char *id = (char *) a;

    cout << "Child loop " << id << " started" << endl;
    for(int i = 0; i < 5; ++i, ++g){
        cout << id << ": " << "i:" << i << " g: " << g << endl;
    }
    thread::yield();
    cout << "Child loop " << id << " ended" << endl;
}

void parent(void *input)
{
    cout << "parent called with arg " << input << endl;
    thread t1 ( (thread_startfunc_t) loop, (void *) "child thread 1");
    thread t2 ( (thread_startfunc_t) loop, (void *) "child thread 2");
    for(int i=0; i<5; i++)
    {
        cout << "Parents loop: " << i << endl;
    }
}

int main() {
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, false, false, 0);
};