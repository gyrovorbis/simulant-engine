
#include <pthread.h>

#ifdef __WIN32__
#include <windows.h>
#elif defined(__PSP__)
#include <pspthreadman.h>
#else
#include <time.h>
#endif

#include "thread.h"

namespace smlt {
namespace thread {

void Thread::join() {
    pthread_join(thread_, nullptr);
}

bool Thread::joinable() const {
    return (ThreadID)thread_ > 0;
}

void Thread::detach() {
    pthread_detach(thread_);
    thread_ = 0;
}

void Thread::exit() {
    int status = 0;
    pthread_exit(&status);
}

void* Thread::thread_runner(void* data) {
    CallableWrapperBase* func = reinterpret_cast<CallableWrapperBase*>(data);
    assert(func);
    if(func) {
        func->call();
        delete func;
    }

    return nullptr;
}

void sleep(size_t ms) {
#ifdef __WIN32__
    Sleep(ms);
#else
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = ms * 1000000;

#ifdef __PSP__
    _S_UNUSED(tim2);
    sceKernelDelayThreadCB(1000000 * tim.tv_sec + (tim.tv_nsec / 1000));
#else
    nanosleep(&tim , &tim2);
#endif

#endif
}

void yield() {
#ifndef SIMULANT_HAS_PTHREAD_YIELD
    /*
     * Massive debates around whether this should be
     * sleep(0) or no-op. My money's on sleep(0).
     */
    sleep(0);
#elif defined(__PSP__)
    /* FIXME: For some reason the CMake check thinks pthread_yield exists? */
    sleep(0);
#else
    pthread_yield();
#endif
}

ThreadID this_thread_id() {
    return (ThreadID) pthread_self();
}

}
}
