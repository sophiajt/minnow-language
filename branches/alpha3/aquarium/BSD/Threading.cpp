// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <cstdlib>
#include <cstdio>

#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <pthread.h>

#include "../Threading.hpp"

/**
 * Platform-specific code for the mutex.  Here, we use what pthread provides.
 */
class Mutex__::Platform_Specific {
public:
    pthread_mutex_t mutex_id;
    pthread_mutexattr_t mutex_attr;
};

/**
 * Create the mutex and allocate platform-specific code
 */
Mutex__::Mutex__() {
    int error_val;

    platform = new Mutex__::Platform_Specific();

    error_val = pthread_mutexattr_init(&platform->mutex_attr);
    if (error_val != 0) {
        std::printf("Internal error: mutex could not be created.  Error no: %i\n", error_val);
        std::exit(-1);
    }

    error_val = pthread_mutex_init(&platform->mutex_id, &platform->mutex_attr);
    if (error_val != 0) {
        std::printf("Internal error: mutex could not be created.  Error no: %i\n", error_val);
        std::exit(-1);
    }
}

/**
 * Delete the mutex, as well as platform-specific resources
 */
Mutex__::~Mutex__() {
    pthread_mutex_destroy(&platform->mutex_id);
    delete (platform);
}

/**
 * Lock the mutex
 */
void Mutex__::lock() {
    pthread_mutex_lock(&platform->mutex_id);
}

/**
 * Unlock the mutex
 */
void Mutex__::unlock() {
    pthread_mutex_unlock(&platform->mutex_id);
}

/**
 * Platform specific inner class for thread, here using pthread
 */
class Thread__::Platform_Specific {
public:
    pthread_t thread_id;

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
    int error_val = pthread_create(&platform->thread_id, NULL, func, NULL);
    if (error_val != 0) {
        std::printf("Internal error: thread could not be created.  Error no: %i\n", error_val);
        std::exit(-1);
    }
}

/**
 * Creates a thread with one argument
 */
void Thread__::create(void*(*func)(void*), void* arg) {
    int error_val = pthread_create(&platform->thread_id, NULL, func, arg);
    if (error_val != 0) {
        std::printf("Internal error: thread could not be create.  Error no: %i\n", error_val);
        std::exit(-1);
    }
}

/**
 * Joins the thread
 */
void Thread__::join() {
    pthread_join(platform->thread_id, NULL);
}

/**
 * Exits and leaves the thread
 */
void Thread__::exit() {
    pthread_exit(NULL);
}

/**
 * Returns the number of hardware (OS level) threads
 */
int num_hw_threads__() {
    int num, return_val;
    size_t size = sizeof(num);

    return_val = sysctlbyname("hw.ncpu", &num, &size, NULL, 0);
    if (return_val == 0) {
        return num;
    }
    else {
        return -1;
    }
}

/**
 * Sleeps in milliseconds
 */
void sleep_in_ms__(unsigned int amount) {
    usleep(amount);
}
