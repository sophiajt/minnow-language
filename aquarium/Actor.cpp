// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <stdio.h>

#include "Actor.hpp"

/**
 * Creates an actor given the size of the total actor (since an actor is the base actor + extra for specific types)
 * @param total_size The size of actor+extensions, likely the sizeof the specific actor
 */
Actor__ *create_actor__(size_t total_size) {
    int i;
    Actor__ *return_val = (Actor__ *)malloc(total_size);
    if (return_val == NULL) {
        printf("Out of memory, exiting...\n");
        exit(-1);
    }

    for (i = 0; i < ACTOR_MSG_CACHE_SIZE__; ++i) {
        return_val->receiver_cache[i] = NULL;
    }
    return_val->cache_id = 0;
    return_val->next_cache_pos = 0;
    return_val->continuation_stack = create_typeless_vector__(sizeof(Type_Union__), 0);
    return_val->exception = NULL;
    return_val->resume_message = NULL;
    return_val->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;

    return return_val;
}

/**
 * Initializes an actor to defaults
 * @param actor The actor to initialize
 */
void initialize_actor__(Actor__ *actor) {
    int i;

    for (i = 0; i < ACTOR_MSG_CACHE_SIZE__; ++i) {
        actor->receiver_cache[i] = NULL;
    }
    actor->cache_id = 0;
    actor->next_cache_pos = 0;
    actor->continuation_stack = create_typeless_vector__(sizeof(Type_Union__), 0);
    actor->exception = NULL;
    actor->resume_message = NULL;
    actor->actor_state = ACTOR_STATE_WAITING_FOR_ACTION__;
}


