// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <cstdio>
#include "../Threading.hpp"

void *loopcount(void *v){
    Thread__ *myThread = (Thread__*)v;
    Mutex__ *mutex = new Mutex__();

    for (int i = 0; i < 15; ++i) {
        Scoped_Lock__ sl(mutex);
        printf("i: %i\n", i);
    }

    delete(mutex);

    myThread->exit();
    return NULL;
}

int main() {
    Thread__ *thread = new Thread__();

    printf("Num processors: %i\n", num_hw_threads__());

    thread->create(loopcount, thread);
    thread->join();
    delete(thread);
}
