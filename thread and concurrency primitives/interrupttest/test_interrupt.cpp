#include <iostream>
#include <cstdlib>
#include <vector>
#include "thread.h"

using std::cout;
using std::endl;
using std::vector;

mutex vectorlock;
vector<int> OrderVector = {};

cv cv1;

void pushnumber(void *a)
{
    char *id = (char *) a;
    int threadid = atoi(id);

    vectorlock.lock();
    OrderVector.push_back(threadid);
    vectorlock.unlock();
}

void pushlast(void *a)
{
    char *id = (char *) a;
    int threadid = atoi(id);

    vectorlock.lock();
    OrderVector.push_back(threadid);
    vectorlock.unlock();

    cv1.signal();
}

void printVector(vector<int> printvec)
{
    cout << "OrderVector elements: ";
    for(size_t i = 0; i < printvec.size(); ++i)
    {
        cout << printvec[i] << " ";
    }
    cout << endl;
}

void parent(void *a)
{
    intptr_t arg = (intptr_t) a;

    cout << "parent called with arg " << arg << endl;
    
    vectorlock.lock();
    while(1)
    {
        thread t1 ( (thread_startfunc_t) pushnumber, (void *) "1");
        thread t2 ( (thread_startfunc_t) pushnumber, (void *) "2");
        thread t3 ( (thread_startfunc_t) pushnumber, (void *) "3");
        thread t4 ( (thread_startfunc_t) pushnumber, (void *) "4");
        thread t5 ( (thread_startfunc_t) pushlast, (void *) "5");

        while(OrderVector.size() != 5)
        {
            cv1.wait(vectorlock);
        }

        vector<int> CorrectOrder = {1, 2, 3, 4, 5};

        if(OrderVector == CorrectOrder)
        {
            //cout << "Ordering Correct" << endl;
            //printVector(OrderVector);
        }
        else
        {
            cout << "Order Incorrect" << endl;
            printVector(OrderVector);
            break;
        }
    }
    vectorlock.unlock();


}

int main()
{
    cpu::boot(1, (thread_startfunc_t) parent, (void *) 100, true, true, 0);
}