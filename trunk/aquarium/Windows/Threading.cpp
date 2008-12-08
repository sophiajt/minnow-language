/*
  Copyright 2008-2008 Jonathan D. Turner. All Rights Reserved.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above
        copyright notice, this list of conditions and the following
        disclaimer in the documentation and/or other materials provided
        with the distribution.
      * Neither the name of Jonathan D. Turner nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <cstdlib>
#include <cstdio>

#include <windows.h>

#include "../Threading.hpp"

/**
 * A wrapper for making WinThreads look like pthreads
 */
struct FunctionPackage {
    void*(*func)(void*); /**< The function pointer */
    void* arg; /**< The argument */

    /**
     * Default constructor
     */
    FunctionPackage(void*(*f)(void*), void* a) {
        func = f;
        arg = a;
    }
};

/**
 * The call WinThreads needs which uses a wrapper to look like pthreads
 */
static DWORD __stdcall ThreadRunner(void *v) {
    FunctionPackage *fp = reinterpret_cast<FunctionPackage*>(v);
    fp->func(fp->arg);
    delete(fp);
}

/**
 * Windows-specific mutex code
 */
class Mutex__::Platform_Specific {
public:
    CRITICAL_SECTION mutex_id;
};

/**
 * Create the mutex and allocate platform-specific code
 */
Mutex__::Mutex__() {
    platform = new Mutex__::Platform_Specific();

    InitializeCriticalSection(&platform->mutex_id);
}

/**
 * Delete the mutex, as well as platform-specific resources
 */
Mutex__::~Mutex__() {
    DeleteCriticalSection(&platform->mutex_id);

    delete (platform);
}

/**
 * Lock the mutex
 */
void Mutex__::lock() {
    EnterCriticalSection(&platform->mutex_id);
}

/**
 * Unlock the mutex
 */
void Mutex__::unlock() {
    LeaveCriticalSection(&platform->mutex_id);
}

/**
 * Platform specific inner class for thread, here using WinThreads
 */
class Thread__::Platform_Specific {
public:
    HANDLE thread_handle;
    DWORD thread_id;
};

/**
 * Thread constructor, which allocates platform-specific resources
 */
Thread__::Thread__() {
    platform = new Thread__::Platform_Specific();
}

/**
 * Thread destructor, which deallocates platform-specific resources
 */
Thread__::~Thread__() {
    delete (platform);
}

/**
 * Creates a thread with no arguments
 */
void Thread__::create(void*(*func)(void*)) {
    FunctionPackage *fp = new FunctionPackage(func, NULL);
    platform->thread_handle = CreateThread(0, 0, ThreadRunner,
            fp, 0, &platform->thread_id);
    if (platform->thread_handle == NULL) {
        std::printf("Internal error: thread could not be created.  Error no: %i\n",
                GetLastError());
        std::exit(-1);
    }
}

/**
 * Creates a thread with one argument
 */
void Thread__::create(void*(*func)(void*), void* arg) {
    FunctionPackage *fp = new FunctionPackage(func, arg);
    platform->thread_handle = CreateThread(0, 0, ThreadRunner,
            fp, 0, &platform->thread_id);
    if (platform->thread_handle == NULL) {
        std::printf("Internal error: thread could not be created.  Error no: %i\n",
                GetLastError());
        std::exit(-1);
    }
}

/**
 * Joins the thread
 */
void Thread__::join() {
    WaitForSingleObject(platform->thread_handle, INFINITE);
}

/**
 * Exits and leaves the thread
 * @todo Needs to be implemented
 */
void Thread__::exit() {

}

/**
 * Returns the number of hardware (OS level) threads
 */
int Thread__::num_hw_threads() {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

/**
 * Sleeps in milliseconds
 */
void sleep_in_ms__(unsigned int amount) {
    Sleep(amount);
}
