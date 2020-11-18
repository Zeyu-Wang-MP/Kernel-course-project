/*
 * thread.h -- public interface to thread library
 *
 * This file should be included by the thread library and by application
 * programs that use the thread library.
 */
#ifndef _THREAD_H
#define _THREAD_H

#include <string>

static const unsigned int STACK_SIZE=262144;  // size of each thread's stack

typedef void (*thread_startfunc_t) (void*);

class thread {
public:
    thread(thread_startfunc_t, void*); // create a new thread
    ~thread();

    void join();                        // wait for this thread to finish

    static void yield();                // yield the CPU

    /*
     * Exchange messages among three threads.  The first thread to call
     * swap3 passes its message to the second thread that calls swap3.
     * The second thread to call swap3 passes its message to the third
     * thread that calls swap3.  The third thread to call swap3 passes
     * its message to the first thread.  After this cycle is complete,
     * the next thread to call swap3 starts a new cycle.  As with other
     * thread functions, swap3 should block only when required by its
     * semantics.
     */
    static std::string swap3(const std::string &);

    class impl;                         // defined by the thread library
    impl* impl_ptr;                     // used by the thread library

    /*
     * Disable the copy constructor and copy assignment operator.
     */
    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

    /*
     * Move constructor and move assignment operator.  Implementing these is
     * optional in Project 2.
     */
    thread(thread&&);
    thread& operator=(thread&&);
};

#include "cpu.h"
#include "mutex.h"
#include "cv.h"

#endif /* _THREAD_H */
