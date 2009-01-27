// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef THREADING_HPP_
#define THREADING_HPP_

#ifdef __cplusplus
/**
 * Internal simple mutex
 */
class Mutex__ {
private:
    class Platform_Specific;
    Platform_Specific *platform;
public:
    Mutex__();
    ~Mutex__();

    /** Locks the mutex, implemented in platform-specific manner */
    void lock();

    /** Unlocks the mutex, implemented in platform-specific manner */
    void unlock();
};

/**
 * A scoped lock, similar to the one found in Boost
 */
class Scoped_Lock__ {
private:
    Mutex__ *mutex;
public:
    /** Constructs a scoped lock associated with the given mutex */
    Scoped_Lock__(Mutex__ *m) {
        mutex = m;
        mutex->lock();
    }
    ~Scoped_Lock__() {
        mutex->unlock();
    }
};

/**
 * Platform-specific code representing a thread
 */
class Thread__ {
private:
    class Platform_Specific;
    Platform_Specific *platform;
public:
    Thread__();
    ~Thread__();

    /** Creates a thread */
    void create(void*(*func)(void*));

    /** Creates a thread, with a single argument */
    void create(void*(*func)(void*), void* arg);

    /** Joins the thread (waits for it to exit and then joins at the parent) */
    void join();

    /** Exits the thread, freeing resources */
    void exit();

};
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Sleeps in a platform-specific way for the given number of milliseconds */
void sleep_in_ms__(unsigned int amount);

/** Returns a count of the number of cores/processors available on the current machine */
int num_hw_threads__();

#ifdef __cplusplus
}
#endif

#endif /* THREADING_HPP_ */
