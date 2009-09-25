// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>
#include <stdlib.h>
#include "../Aquarium.hpp"

typedef struct {
    unsigned int exception_id;
} Exception;

void throw_exception(Actor__ *actor__) {
    //to throw an exception:
    //1) create it
    Exception *e = (Exception*)malloc(sizeof(Exception));
    //2) initialize it
    e->exception_id = 123;
    //3) exhaust the remaining timeslice, as this shortcuts the slice and makes
    //us react to the exception
    actor__->timeslice_remaining = 0;
    //4) set the exception in the actor
    actor__->exception = e;
    //5) return immediately
    return;
}

BOOL main_action(Message__ *m) {
    Actor__ *actor__ = (Actor__*)m->recipient;

    unsigned int cont_id__ = 0;

    if (actor__->continuation_stack->current_size > 0) {
        cont_id__ = INDEX_AT__(actor__->continuation_stack, actor__->continuation_stack->current_size - 1, unsigned int);
        pop_off_typeless_vector__(actor__->continuation_stack);
    }

    switch (cont_id__) {
        case (0) : {
            printf("Throwing exception:\n");
            throw_exception(actor__);
            if (actor__->timeslice_remaining == 0) {
                if (actor__->exception != NULL) {
                    printf("SUCCESS: exception caught: %i\n", ((Exception*)(actor__->exception))->exception_id);
                    exit(0);
                }
            }
            printf("FAIL: Exception not caught successfully.\n");
        }
    }

    exit(0);
}
int main(int argc, char *argv[]) {
    aquarium_main__(argc, argv, main_action, FALSE);

    return 0;
}
